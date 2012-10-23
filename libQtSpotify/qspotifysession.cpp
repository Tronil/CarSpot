/****************************************************************************
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Yoann Lopes (yoann.lopes@nokia.com)
**
** This file is part of the MeeSpot project.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** Redistributions of source code must retain the above copyright notice,
** this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its
** contributors may be used to endorse or promote products derived from
** this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
** FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
****************************************************************************/
/****************************************************************************
**
** Copyright (c) 2012 Troels Nilsson - Various modifications and fixes.
**
****************************************************************************/

#include "qspotifysession.h"
#include "qspotifyuser.h"
#include "qspotifytrack.h"
#include "qspotifytracklist.h"
#include "qspotifyplayqueue.h"
#include "qspotifyalbum.h"
#include "qspotifyartist.h"
#include "qspotifyartistbrowse.h"
#include "spotify_key.h"
#include "qspotifyradio.h"
#include "qspotifyplaylist.h"
#include "qspotifyplaylistcontainer.h"

#include <QtCore/QHash>
#include <QtCore/QEvent>
#include <QtCore/QCoreApplication>
#include <QtMultimediaKit/QAudioOutput>
#include <QtCore/QIODevice>
#include <QtCore/QBuffer>
#include <QtCore/QMutexLocker>
#include <QtCore/QDebug>
#include <QtGui/QDesktopServices>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QtSystemInfo/QSystemStorageInfo>
#include <QSystemDeviceInfo>

QTM_USE_NAMESPACE

#define BUFFER_SIZE 409600
#define AUDIOSTREAM_UPDATE_INTERVAL 20

class QSpotifyAudioThreadWorker;

static QBuffer g_buffer;
static QMutex g_mutex;
static int g_readPos = 0;
static int g_writePos = 0;
static bool g_endOfTrack= false;
static QSpotifyAudioThreadWorker *g_audioWorker;

static QMutex g_imageRequestMutex;
static QHash<QString, QWaitCondition *> g_imageRequestConditions;
static QHash<QString, QImage> g_imageRequestImages;
static QHash<sp_image *, QString> g_imageRequestObject;

QSpotifySession *QSpotifySession::m_instance = 0;


class QSpotifyAudioThreadWorker : public QObject
{
public:
    QSpotifyAudioThreadWorker();

    bool event(QEvent *);

    enum Events {
        StreamingStartedEvent = QEvent::User,
        ResumeEvent,
        SuspendEvent,
        StopEvent,
        ResetBuffersEvent
    };

private:
    void startStreaming(int channels, int sampleRate);
    void updateAudioBuffer();

    QAudioOutput *m_audioOutput;
    QIODevice *m_iodevice;
    int m_audioTimerID;
    int m_timeCounter;
    int m_previousElapsedTime;
};

class QSpotifyConnectionErrorEvent : public QEvent
{
public:
    QSpotifyConnectionErrorEvent(sp_error error)
        : QEvent(Type(QSpotifySession::ConnectionErrorEvent))
        , m_error(error)
    { }

    sp_error error() const { return m_error; }

private:
    sp_error m_error;
};


class QSpotifyStreamingStartedEvent : public QEvent
{
public:
    QSpotifyStreamingStartedEvent(int channels, int sampleRate)
        : QEvent(Type(QSpotifyAudioThreadWorker::StreamingStartedEvent))
        , m_channels(channels)
        , m_sampleRate(sampleRate)
    { }

    int channels() const { return m_channels; }
    int sampleRate() const { return m_sampleRate; }

private:
    int m_channels;
    int m_sampleRate;
};


class QSpotifyTrackProgressEvent : public QEvent
{
public:
    QSpotifyTrackProgressEvent(int delta)
        : QEvent(Type(QSpotifySession::TrackProgressEvent))
        , m_delta(delta)
    { }

    int delta() const { return m_delta; }

private:
    int m_delta;
};

class QSpotifyRequestImageEvent : public QEvent
{
public:
    QSpotifyRequestImageEvent(const QString &id)
        : QEvent(Type(QSpotifySession::RequestImageEvent))
        , m_id(id)
    { }

    QString imageId() const { return m_id; }

private:
    QString m_id;
};

class QSpotifyReceiveImageEvent : public QEvent
{
public:
    QSpotifyReceiveImageEvent(sp_image *image)
        : QEvent(Type(QSpotifySession::ReceiveImageEvent))
        , m_image(image)
    { }

    sp_image *image() const { return m_image; }

private:
    sp_image *m_image;
};

class QSpotifyOfflineErrorEvent : public QEvent
{
public:
    QSpotifyOfflineErrorEvent(sp_error error)
        : QEvent(Type(QSpotifySession::OfflineErrorEvent))
        , m_error(error)
    { }

    sp_error error() const { return m_error; }

private:
    sp_error m_error;
};

QSpotifyAudioThreadWorker::QSpotifyAudioThreadWorker()
    : QObject()
    , m_audioOutput(0)
    , m_iodevice(0)
    , m_audioTimerID(0)
    , m_timeCounter(0)
    , m_previousElapsedTime(0)
{
    QMutexLocker lock(&g_mutex);
    g_endOfTrack = false;
}

bool QSpotifyAudioThreadWorker::event(QEvent *e)
{
    if (e->type() == QEvent::Type(StreamingStartedEvent)) {
        // StreamingStarted Event
        QSpotifyStreamingStartedEvent *ev = static_cast<QSpotifyStreamingStartedEvent *>(e);
        startStreaming(ev->channels(), ev->sampleRate());
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(ResumeEvent)) {
        // Resume
        if (m_audioOutput) {
            m_audioTimerID = startTimer(AUDIOSTREAM_UPDATE_INTERVAL);
            m_audioOutput->resume();
        }
        g_mutex.lock();
        g_endOfTrack = false;
        g_mutex.unlock();
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(SuspendEvent)) {
        // Suspend
        if (m_audioOutput) {
            killTimer(m_audioTimerID);
            m_audioOutput->suspend();
        }
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(StopEvent)) {
        // Stop
        QMutexLocker lock(&g_mutex);
        killTimer(m_audioTimerID);
        g_buffer.close();
        g_buffer.setData(QByteArray());
        g_readPos = 0;
        g_writePos = 0;
        if (m_audioOutput) {
            m_audioOutput->suspend();
            m_audioOutput->stop();
            delete m_audioOutput;
            m_audioOutput = 0;
            m_iodevice = 0;
        }
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(ResetBuffersEvent)) {
        // Reset buffers
        if (m_audioOutput) {
            QMutexLocker lock(&g_mutex);
            killTimer(m_audioTimerID);
            m_audioOutput->suspend();
            m_audioOutput->stop();
            g_buffer.close();
            g_buffer.setData(QByteArray());
            g_buffer.open(QIODevice::ReadWrite);
            g_readPos = 0;
            g_writePos = 0;
            g_endOfTrack = false;
            m_audioOutput->reset();
            m_iodevice = m_audioOutput->start();
            m_audioOutput->suspend();
            m_audioTimerID = startTimer(AUDIOSTREAM_UPDATE_INTERVAL);
            m_timeCounter = 0;
            m_previousElapsedTime = 0;
        }
        e->accept();
        return true;
    } else if (e->type() == QEvent::Timer) {
        QTimerEvent *te = static_cast<QTimerEvent *>(e);
        if (te->timerId() == m_audioTimerID) {
            updateAudioBuffer();
            e->accept();
            return true;
        }
    }
    return QObject::event(e);
}

void QSpotifyAudioThreadWorker::startStreaming(int channels, int sampleRate)
{
    if (!m_audioOutput) {
        QAudioFormat af;
        af.setChannelCount(channels);
        af.setCodec(QLatin1String("audio/pcm"));
        af.setSampleRate(sampleRate);
        af.setSampleSize(16);
        af.setSampleType(QAudioFormat::SignedInt);

        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
        if (!info.isFormatSupported(af)) {
            qWarning()<<"raw audio format not supported by backend, cannot play audio.";
            QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(QSpotifySession::AudioFormatNotSupportedEvent)));
            return;
        }

        m_audioOutput = new QAudioOutput(af);
        m_audioOutput->setBufferSize(BUFFER_SIZE);
        m_iodevice = m_audioOutput->start();
        m_audioOutput->suspend();
        m_audioTimerID = startTimer(AUDIOSTREAM_UPDATE_INTERVAL);
        g_mutex.lock();
        g_endOfTrack = false;
        g_mutex.unlock();
        m_timeCounter = 0;
        m_previousElapsedTime = 0;
    }
}

void QSpotifyAudioThreadWorker::updateAudioBuffer()
{
    if (!m_audioOutput)
        return;

    if (m_audioOutput->state() == QAudio::SuspendedState)
        m_audioOutput->resume();

    g_mutex.lock();
    if (g_endOfTrack && m_audioOutput->state() == QAudio::IdleState) {
        g_mutex.unlock();
        killTimer(m_audioTimerID);
        int elapsedTime = int(m_audioOutput->processedUSecs() / 1000);
        QCoreApplication::postEvent(QSpotifySession::instance(), new QSpotifyTrackProgressEvent(elapsedTime - m_previousElapsedTime));
        QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(QSpotifySession::EndOfTrackEvent)));
        m_previousElapsedTime = elapsedTime;
        return;
    } else {

        int toRead = qMin(g_writePos - g_readPos, m_audioOutput->bytesFree());
        g_buffer.seek(g_readPos);
        char data[toRead];
        int read =  g_buffer.read(&data[0], toRead);
        g_readPos += read;
        g_mutex.unlock();

        m_iodevice->write(&data[0], read);

    }

    m_timeCounter += AUDIOSTREAM_UPDATE_INTERVAL;
    if (m_timeCounter >= 1000) {
        m_timeCounter = 0;
        int elapsedTime = int(m_audioOutput->processedUSecs() / 1000);
        QCoreApplication::postEvent(QSpotifySession::instance(), new QSpotifyTrackProgressEvent(elapsedTime - m_previousElapsedTime));
        m_previousElapsedTime = elapsedTime;
    }
}


class QSpotifyAudioThread : public QThread
{
public:
    void run();
};

void QSpotifyAudioThread::run()
{
    g_audioWorker = new QSpotifyAudioThreadWorker;
    exec();
    delete g_audioWorker;
}


static void callback_logged_in(sp_session *, sp_error error)
{
    QCoreApplication::postEvent(QSpotifySession::instance(), new QSpotifyConnectionErrorEvent(error));
    if (error == SP_ERROR_OK)
        QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(QSpotifySession::LoggedInEvent)));
}

static void callback_logged_out(sp_session *)
{
    QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(QSpotifySession::LoggedOutEvent)));
}

static void callback_connection_error(sp_session *, sp_error error)
{
    QCoreApplication::postEvent(QSpotifySession::instance(), new QSpotifyConnectionErrorEvent(error));
}

static void callback_notify_main_thread(sp_session *)
{
    QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(QSpotifySession::NotifyMainThreadEvent)));
}

static void callback_metadata_updated(sp_session *)
{
    QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(QSpotifySession::MetaDataUpdatedEvent)));
}

static void callback_userinfo_updated(sp_session* )
{
    QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(QSpotifySession::MetaDataUpdatedEvent)));
}

static int callback_music_delivery(sp_session *, const sp_audioformat *format, const void *frames, int num_frames)
{
    if (num_frames == 0)
        return 0;

    QMutexLocker locker(&g_mutex);

    if (g_endOfTrack)
        return 0;

    if (!g_buffer.isOpen()) {
        g_buffer.open(QIODevice::ReadWrite);
        QCoreApplication::postEvent(g_audioWorker,
                                    new QSpotifyStreamingStartedEvent(format->channels, format->sample_rate));
    }

    int availableFrames = (BUFFER_SIZE - (g_writePos - g_readPos)) / (sizeof(int16_t) * format->channels);
    int writtenFrames = qMin(num_frames, availableFrames);

    if (writtenFrames == 0)
        return 0;

    g_buffer.seek(g_writePos);
    g_writePos += g_buffer.write((const char *) frames, writtenFrames * sizeof(int16_t) * format->channels);

    return writtenFrames;
}

static void callback_end_of_track(sp_session *)
{
    QMutexLocker lock(&g_mutex);
    g_endOfTrack = true;
}

static void callback_play_token_lost(sp_session *)
{
    QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(QSpotifySession::PlayTokenLostEvent)));
}

static void callback_log_message(sp_session *, const char *data)
{
    fprintf(stderr, data);
}

static void callback_offline_error(sp_session *, sp_error error)
{
    if (error != SP_ERROR_OK)
        QCoreApplication::postEvent(QSpotifySession::instance(), new QSpotifyOfflineErrorEvent(error));
}

static void callback_connectionstate_updated(sp_session *)
{
    QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(QSpotifySession::ConnectionStateUpdatedEvent)));
}

QSpotifySession::QSpotifySession()
    : QObject(0)
    , m_timerID(0)
    , m_connectionStatus(LoggedOut)
    , m_connectionError(ConnectionOk)
    , m_offlineError(OfflineOk)
    , m_connectionRules(AllowSyncOverWifi | AllowNetworkIfRoaming)
    , m_streamingQuality(Unknown)
    , m_syncQuality(Unknown)
    , m_syncOverMobile(false)
    , m_user(0)
    , m_pending_connectionRequest(false)
    , m_isLoggedIn(false)
    , m_explicitLogout(false)
    , m_offlineMode(false)
    , m_forcedOfflineMode(false)
    , m_ignoreNextConnectionError(false)
    , m_playQueue(new QSpotifyPlayQueue)
    , m_currentTrack(0)
    , m_isPlaying(false)
    , m_currentTrackPosition(0)
    , m_currentTrackPlayedDuration(0)
    , m_shuffle(false)
    , m_repeat(false)
    , m_radioMode(false)
    , m_radio(0)
{
    m_networkConfManager = new QNetworkConfigurationManager;
    connect(m_networkConfManager, SIGNAL(onlineStateChanged(bool)), this, SLOT(onOnlineChanged()));
    connect(m_networkConfManager, SIGNAL(onlineStateChanged(bool)), this, SIGNAL(isOnlineChanged()));
    connect(m_networkConfManager, SIGNAL(configurationChanged(QNetworkConfiguration)), this, SIGNAL(isOnlineChanged()));
    connect(m_networkConfManager, SIGNAL(configurationChanged(QNetworkConfiguration)), this, SLOT(configurationChanged()));

    m_audioThread = new QSpotifyAudioThread;
    m_audioThread->start(QThread::HighestPriority);

    // Resource management stuff
    m_resourceSet = new ResourcePolicy::ResourceSet(QLatin1String("player"), 0, false, false);
    m_audioResource = new ResourcePolicy::AudioResource(QLatin1String("player"));
    m_audioResource->setProcessID(QCoreApplication::applicationPid());
    m_audioResource->setStreamTag(QLatin1String("media.name"), QLatin1String("*"));
    m_audioResource->setOptional(false);
    m_resourceSet->addResourceObject(m_audioResource);
    connect(m_resourceSet, SIGNAL(resourcesGranted(QList<ResourcePolicy::ResourceType>)), this, SLOT(resourceAcquiredHandler(QList<ResourcePolicy::ResourceType>)));
    connect(m_resourceSet, SIGNAL(lostResources()), this, SLOT(resourceLostHandler()));
    connect(m_resourceSet, SIGNAL(resourcesReleasedByManager()), this, SLOT(resourceReleasedByManager()));
}

void QSpotifySession::init()
{
    QSystemDeviceInfo device_info;
    char *imei;
    imei = new char[device_info.imei().size() + 1];
    strcpy(imei, device_info.imei().toLocal8Bit());

    m_sp_callbacks.logged_in = callback_logged_in;
    m_sp_callbacks.logged_out = callback_logged_out;
    m_sp_callbacks.metadata_updated = callback_metadata_updated;
    m_sp_callbacks.connection_error = callback_connection_error;
    m_sp_callbacks.message_to_user = 0;
    m_sp_callbacks.notify_main_thread = callback_notify_main_thread;
    m_sp_callbacks.music_delivery = callback_music_delivery;
    m_sp_callbacks.play_token_lost = callback_play_token_lost;
    m_sp_callbacks.log_message = callback_log_message;
    m_sp_callbacks.end_of_track = callback_end_of_track;
    m_sp_callbacks.streaming_error = 0;
    m_sp_callbacks.userinfo_updated = callback_userinfo_updated;
    m_sp_callbacks.start_playback = 0;
    m_sp_callbacks.stop_playback = 0;
    m_sp_callbacks.get_audio_buffer_stats = 0;
    m_sp_callbacks.offline_status_updated = 0;
    m_sp_callbacks.offline_error = callback_offline_error;
    m_sp_callbacks.credentials_blob_updated = 0;
    m_sp_callbacks.connectionstate_updated = callback_connectionstate_updated;
    m_sp_callbacks.scrobble_error = 0;
    m_sp_callbacks.private_session_mode_changed = 0;

    m_sp_config.api_version = SPOTIFY_API_VERSION;
    m_sp_config.cache_location = "/home/user/MyDocs/.carspot";
    m_sp_config.settings_location = "/home/user/MyDocs/.carspot";
    m_sp_config.application_key = g_appkey;
    m_sp_config.application_key_size = g_appkey_size;
    m_sp_config.user_agent = "CarSpot";
    m_sp_config.callbacks = &m_sp_callbacks;
    m_sp_config.userdata = 0;
    m_sp_config.compress_playlists = false;
    m_sp_config.dont_save_metadata_for_playlists = false;
    m_sp_config.initially_unload_playlists = false;
    m_sp_config.device_id = imei;
    m_sp_config.proxy = 0;
    m_sp_config.proxy_username = 0;
    m_sp_config.proxy_password = 0;
    m_sp_config.ca_certs_filename = 0;
    m_sp_config.tracefile = 0;

    sp_error error = sp_session_create(&m_sp_config, &m_sp_session);

    if (error != SP_ERROR_OK) {
        fprintf(stderr, "failed to create session: %s\n",
                sp_error_message(error));
    } else {
        QtMobility::QSystemStorageInfo storageInfo;
        qlonglong totalSpace = storageInfo.totalDiskSpace(QString::fromLatin1(m_sp_config.cache_location));
        sp_session_set_cache_size(m_sp_session, totalSpace / 1000000 - 1000);

        QSettings settings;

        m_offlineMode = settings.value(QLatin1String("offlineMode"), false).toBool();

        checkNetworkAccess();

        StreamingQuality quality = StreamingQuality(settings.value(QLatin1String("streamingQuality"), int(LowQuality)).toInt());
        setStreamingQuality(quality);

        StreamingQuality syncQuality = StreamingQuality(settings.value(QLatin1String("syncQuality"), int(HighQuality)).toInt());
        setSyncQuality(syncQuality);

        bool syncMobile = settings.value(QLatin1String("syncOverMobile"), false).toBool();
        setSyncOverMobile(syncMobile);

        QString storedLogin = getStoredLoginInformation();
        if (!storedLogin.isEmpty()) {
            login(storedLogin);
        }

        bool shuffle = settings.value(QLatin1String("shuffle"), false).toBool();
        setShuffle(shuffle);

        bool repeat = settings.value(QLatin1String("repeat"), false).toBool();
        setRepeat(repeat);

        connect(this, SIGNAL(offlineModeChanged()), m_playQueue, SLOT(onOfflineModeChanged()));

        int i = 0;
        while (settings.value(QLatin1String("Last Artist ") + QString::number(i), 0) != 0 && i < MAX_LAST_ARTISTS)
        {
            QSpotifyArtist *artist = QSpotifyArtist::fromLink(settings.value(QLatin1String("Last Artist ") + QString::number(i), 0).toString());

            if (artist)
            {
                m_lastArtists.append(artist);
                connect(artist, SIGNAL(artistDataChanged()), this, SIGNAL(lastArtistsChanged()));
            }

            ++i;
        }
    }
}

QSpotifySession::~QSpotifySession()
{
    cleanUp();
    m_instance = 0;
}

QSpotifySession *QSpotifySession::instance()
{
    if (!m_instance) {
        m_instance = new QSpotifySession;
        m_instance->init();
    }
    return m_instance;
}

void QSpotifySession::cleanUp()
{
    saveState();

    stop();
    m_audioThread->quit();
    m_audioThread->wait();
    delete m_audioThread;

    logout(true);

    delete m_user;
    delete m_playQueue;
    delete m_resourceSet;
    delete m_networkConfManager;
    if (m_radio)
        delete m_radio;
    qDeleteAll(m_lastArtists);
    m_lastArtists.clear();

    sp_session_release(m_sp_session);
}

bool QSpotifySession::event(QEvent *e)
{
    if (e->type() == QEvent::Type(NotifyMainThreadEvent)) {
        processSpotifyEvents();
        e->accept();
        return true;
    } else if (e->type() == QEvent::Timer) {
        QTimerEvent *te = static_cast<QTimerEvent *>(e);
        if (te->timerId() == m_timerID) {
            processSpotifyEvents();
            e->accept();
            return true;
        }
    } else if (e->type() == QEvent::Type(ConnectionErrorEvent)) {
        // ConnectionError event
        QSpotifyConnectionErrorEvent *ev = static_cast<QSpotifyConnectionErrorEvent *>(e);
        setConnectionError(ConnectionError(ev->error()));
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(MetaDataUpdatedEvent)) {
        // Metadata event
        emit metadataUpdated();
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(EndOfTrackEvent)) {
        // End of track event
        playNext();
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(AudioFormatNotSupportedEvent)) {
        stop();
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(TrackProgressEvent)) {
        // Track progressed
        QSpotifyTrackProgressEvent *ev = static_cast<QSpotifyTrackProgressEvent *>(e);
        m_currentTrackPosition += ev->delta();
        m_currentTrackPlayedDuration += ev->delta();
        emit currentTrackPositionChanged();
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(RequestImageEvent)) {
        QSpotifyRequestImageEvent *ev = static_cast<QSpotifyRequestImageEvent *>(e);
        sendImageRequest(ev->imageId());
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(ReceiveImageEvent)) {
        QSpotifyReceiveImageEvent *ev = static_cast<QSpotifyReceiveImageEvent *>(e);
        receiveImageResponse(ev->image());
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(PlayTokenLostEvent)) {
        // Play Token Lost
        emit playTokenLost();
        pause();
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(LoggedInEvent)) {
        // LoggedIn
        onLoggedIn();
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(LoggedOutEvent)) {
        // LoggedOut
        onLoggedOut();
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(OfflineErrorEvent)) {
        // Offline error
        QSpotifyOfflineErrorEvent *ev = static_cast<QSpotifyOfflineErrorEvent *>(e);
        m_offlineError = OfflineError(ev->error());
        m_offlineErrorMessage = getErrorMessage(m_offlineError);
        emit offlineErrorChanged();
        e->accept();
        return true;
    } else if (e->type() == QEvent::Type(ConnectionStateUpdatedEvent)) {
        // update connection state
        setConnectionStatus(ConnectionStatus(sp_session_connectionstate(m_sp_session)));
        if (m_offlineMode && m_connectionStatus == LoggedIn) {
            setConnectionRule(AllowNetwork, true);
            setConnectionRule(AllowNetwork, false);
        }
    }

    return QObject::event(e);
}

void QSpotifySession::processSpotifyEvents()
{
    if (m_timerID)
        killTimer(m_timerID);
    int nextTimeout;
    do {
        sp_session_process_events(m_sp_session, &nextTimeout);
    } while (nextTimeout == 0);

    m_timerID = startTimer(nextTimeout);
}

void QSpotifySession::setStreamingQuality(StreamingQuality q)
{
    if (m_streamingQuality == q)
        return;

    m_streamingQuality = q;
    QSettings s;
    s.setValue(QLatin1String("streamingQuality"), int(q));
    sp_session_preferred_bitrate(m_sp_session, sp_bitrate(q));

    emit streamingQualityChanged();
}

void QSpotifySession::setSyncQuality(StreamingQuality q)
{
    if (m_syncQuality == q)
        return;

    m_syncQuality = q;
    QSettings s;
    s.setValue(QLatin1String("syncQuality"), int(q));
    sp_session_preferred_offline_bitrate(m_sp_session, sp_bitrate(q), false);

    emit syncQualityChanged();
}

void QSpotifySession::onLoggedIn()
{
    if (m_user)
        return;

    m_isLoggedIn = true;
    m_user = new QSpotifyUser(sp_session_user(m_sp_session));

    m_pending_connectionRequest = false;
    emit pendingConnectionRequestChanged();
    emit isLoggedInChanged();

    checkNetworkAccess();

    if (m_lastArtists.count() == 0)
    {
        // No played artist data, try to populate from the users starred list, then other playlists
        if (m_user->isLoaded())
        {
            userLoaded();
        }
        else
        {
            connect(m_user, SIGNAL(isLoadedChanged()), this, SLOT(userLoaded()));
        }
    }

    if (m_keepResources)
    {
        m_resourceSet->acquire();
    }

    restoreState();
}

void QSpotifySession::onLoggedOut()
{
    if (!m_explicitLogout)
        return;

    delete m_user;
    m_user = 0;
    m_explicitLogout = false;
    m_isLoggedIn = false;

    m_pending_connectionRequest = false;
    emit pendingConnectionRequestChanged();
    emit isLoggedInChanged();
}

void QSpotifySession::setConnectionStatus(ConnectionStatus status)
{
    if (m_connectionStatus == status)
        return;

    m_connectionStatus = status;
    emit connectionStatusChanged();
}

void QSpotifySession::setConnectionError(ConnectionError error)
{
    if (error == ConnectionOk || m_offlineMode)
        return;

    if (m_pending_connectionRequest) {
        m_pending_connectionRequest = false;
        emit pendingConnectionRequestChanged();
    }

    if (error == UnableToContactServer && m_ignoreNextConnectionError) {
        m_ignoreNextConnectionError = false;
        return;
    }

    if (error == UnableToContactServer || error == NetworkDisabled) {
        setOfflineMode(true, true);
    }

    m_connectionError = error;
    m_connectionErrorMessage = getErrorMessage(error);
    if (error != ConnectionOk && m_currentTrack && !m_currentTrack->isAvailableOffline())
        pause();
    emit connectionErrorChanged();
}

void QSpotifySession::login(const QString &username, const QString &password)
{
    if (!isValid() || m_isLoggedIn || m_pending_connectionRequest)
        return;

    m_pending_connectionRequest = true;
    emit pendingConnectionRequestChanged();
    emit loggingIn();

    if (password.isEmpty())
        sp_session_relogin(m_sp_session);
    else
        sp_session_login(m_sp_session, username.toUtf8().constData(), password.toUtf8().constData(), true, 0);
}

void QSpotifySession::logout(bool keepLoginInfo)
{
    if (!m_isLoggedIn || m_pending_connectionRequest)
        return;

    stop();
    m_playQueue->clear();

    if (!keepLoginInfo) {
        setOfflineMode(false);
        sp_session_forget_me(m_sp_session);
    }

    m_explicitLogout = true;

    m_pending_connectionRequest = true;
    emit pendingConnectionRequestChanged();
    emit loggingOut();
    sp_session_logout(m_sp_session);
}

void QSpotifySession::setShuffle(bool s)
{
    if (m_shuffle == s)
        return;

    QSettings settings;
    settings.setValue(QLatin1String("shuffle"), s);
    m_playQueue->setShuffle(s);
    m_shuffle = s;
    emit shuffleChanged();
}

void QSpotifySession::setRepeat(bool r)
{
    if (m_repeat == r)
        return;

    QSettings s;
    s.setValue(QLatin1String("repeat"), r);
    m_playQueue->setRepeat(r);
    m_repeat = r;
    emit repeatChanged();
}

bool QSpotifySession::setCurrentTrack(QSpotifyTrack *track)
{
    if (track->error() != QSpotifyTrack::Ok || !track->isAvailable() || m_currentTrack == track)
        return false;

    if (m_currentTrack)
        stop(true);

    if (!track->seen())
        track->setSeen(true);

    sp_error error = sp_session_player_load(m_sp_session, track->m_sp_track);
    if (error != SP_ERROR_OK) {
        fprintf(stderr, "failed to load track: %s\n",
                sp_error_message(error));
        return false;
    }

    m_currentTrack = track;
    m_currentTrackPosition = 0;
    m_currentTrackPlayedDuration = 0;
    emit currentTrackChanged();
    emit currentTrackPositionChanged();

    if (!m_radioMode)
    {
        updateLastArtists(track->artistObject());
    }

    return true;
}

void QSpotifySession::play(QSpotifyTrack *track)
{
    if (!setCurrentTrack(track))
        return;

    m_isPlaying = true;

    if (m_resourceSet->hasResourcesGranted())
    {
        // HACK: As of libspotify 12.1.51 it seems calling (un)load and play too fast after
        // eachother causes the stack to be overwritten in the audio thread in some cases.
        // To avoid this reaquire the audio resource, instead of calling beginPlayBack() directly
        m_resourceSet->release();
    }

    m_resourceSet->acquire();

    emit isPlayingChanged();
}

void QSpotifySession::beginPlayBack()
{
    sp_session_player_play(m_sp_session, true);

    QCoreApplication::postEvent(g_audioWorker, new QEvent(QEvent::Type(QSpotifyAudioThreadWorker::ResumeEvent)));
}

void QSpotifySession::pause()
{
    if (!m_isPlaying)
        return;

    sp_session_player_play(m_sp_session, false);
    m_isPlaying = false;
    emit isPlayingChanged();

    QCoreApplication::postEvent(g_audioWorker, new QEvent(QEvent::Type(QSpotifyAudioThreadWorker::SuspendEvent)));

    if (!m_keepResources)
    {
        m_resourceSet->release();
    }
}

void QSpotifySession::resume()
{
    if (m_isPlaying || !m_currentTrack)
        return;

    m_isPlaying = true;

    if (m_resourceSet->hasResourcesGranted())
    {
        // HACK: As of libspotify 12.1.51 it seems calling (un)load and play too fast after
        // eachother causes the stack to be overwritten in the audio thread in some cases.
        // To avoid this reaquire the audio resource, instead of calling beginPlayBack() directly
        m_resourceSet->release();
    }

    m_resourceSet->acquire();

    emit isPlayingChanged();
}

void QSpotifySession::stop(bool dontEmitSignals)
{
    if (!m_isPlaying && !m_currentTrack)
        return;

    sp_session_player_unload(m_sp_session);
    m_isPlaying = false;
    m_currentTrack = 0;
    m_currentTrackPosition = 0;
    m_currentTrackPlayedDuration = 0;

    if (!dontEmitSignals) {
         emit isPlayingChanged();
         emit currentTrackChanged();
         emit currentTrackPositionChanged();
    }

    QCoreApplication::postEvent(g_audioWorker, new QEvent(QEvent::Type(QSpotifyAudioThreadWorker::StopEvent)));

    if (!m_keepResources)
    {
        m_resourceSet->release();
    }
}

void QSpotifySession::seek(int offset)
{
    if (!m_currentTrack)
        return;

    sp_session_player_seek(m_sp_session, offset);

    m_currentTrackPosition = offset;
    emit currentTrackPositionChanged();

    QCoreApplication::postEvent(g_audioWorker, new QEvent(QEvent::Type(QSpotifyAudioThreadWorker::ResetBuffersEvent)));
}

void QSpotifySession::playNext()
{
    if (m_radioMode)
        m_radio->nextSong();
    else
        m_playQueue->next();
}

void QSpotifySession::playPrevious()
{
    if (!m_radioMode)
    {
        m_playQueue->previous();
    }
}

void QSpotifySession::enqueue(QSpotifyTrack *track)
{
    if (m_radioMode)
    {
        setRadioMode(false);
    }

    m_playQueue->enqueueTrack(track);
}

void QSpotifySession::startRadio(QSpotifyArtist *artist)
{
    if (!m_offlineMode)
    {
        setRadioMode(true);

        if (m_radio)
        {
            delete m_radio;
        }

        m_radio = new QSpotifyRadio(*artist);
        connect(m_radio, SIGNAL(busyChanged()), this, SIGNAL(radioBusyChanged()));
        connect(m_radio, SIGNAL(errorOccured()), this, SLOT(radioErrorOccurred()));

        m_radio->nextSong();
    }
}

void QSpotifySession::setRadioMode(bool mode)
{
    if (mode == m_radioMode)
        return;

    if (mode == false)
    {
        delete m_radio;
        m_radio = 0;
    }

    m_radioMode = mode;
    emit radioModeChanged();
}

bool QSpotifySession::radioBusy() const
{
    if (m_radio)
    {
        return m_radio->busy();
    }
    else
    {
        return false;
    }
}

void QSpotifySession::radioErrorOccurred()
{
    // Stop radio playback
    stop();
    setRadioMode(false);

    // Radio errors are handled as a connection error
    setConnectionError(RadioError);
}

void QSpotifySession::updateLastArtists(QSpotifyArtist *new_artist)
{
    int i = 0;
    bool artist_found = false;
    QSettings settings;

    // Check if the artist is already in the list
    while (i < m_lastArtists.count())
    {
        if (m_lastArtists[i]->spartist() == new_artist->spartist())
        {
            artist_found = true;
            // Move to front of list
            m_lastArtists.move(i, 0);
            break;
        }
        ++i;
    }

    if (!artist_found)
    {
        QSpotifyArtist *artist = new QSpotifyArtist(new_artist->spartist());
        m_lastArtists.prepend(artist);
        connect(artist, SIGNAL(artistDataChanged()), this, SIGNAL(lastArtistsChanged()));

        if (m_lastArtists.count() > MAX_LAST_ARTISTS)
        {
            delete(m_lastArtists.last());
            m_lastArtists.removeLast();
        }
    }

    // Save the list to permanent storage
    for (i = 0; i < m_lastArtists.count(); i++)
    {
        settings.setValue(QLatin1String("Last Artist ") + QString::number(i), m_lastArtists[i]->asLink());
    }

    emit lastArtistsChanged();
}

QList<QObject*> QSpotifySession::lastArtists() const
{
    QList<QObject*> list;

    for (int i = 0; i < m_lastArtists.count(); i++)
    {
        list.append(static_cast<QObject*>(m_lastArtists[i]));
    }

    return list;
}

void QSpotifySession::resourceAcquiredHandler(const QList<ResourcePolicy::ResourceType> &)
{
    if (m_isPlaying)
    {
        beginPlayBack();
    }
}

void QSpotifySession::resourceLostHandler()
{
    // Audio resource temporarily unavailable, wait for it to become available again
    sp_session_player_play(m_sp_session, false);

    QCoreApplication::postEvent(g_audioWorker, new QEvent(QEvent::Type(QSpotifyAudioThreadWorker::SuspendEvent)));
}

void QSpotifySession::resourceReleasedByManager()
{
    // Audio resource permanently lost
    pause();

    if (m_keepResources)
    {
        // Attempt to reacquire immediately
        m_resourceSet->acquire();
    }
}

QString QSpotifySession::formatDuration(qint64 d) const
{
    d /= 1000;
    int s = d % 60;
    d /= 60;
    int m = d % 60;
    int h = d / 60;

    QString r;
    if (h > 0)
        r += QString::number(h) + QLatin1String(":");
    r += QLatin1String(m > 9 || h == 0 ? "" : "0") + QString::number(m) + QLatin1String(":");
    r += QLatin1String(s > 9 ? "" : "0") + QString::number(s);

    return r;
}

QString QSpotifySession::getStoredLoginInformation() const
{
    QString username;
    char buffer[200];
    int size = sp_session_remembered_user(m_sp_session, &buffer[0], 200);
    if (size > 0) {
        username = QString::fromUtf8(&buffer[0], size);
    }
    return username;
}

QImage QSpotifySession::requestSpotifyImage(const QString &id)
{
    g_imageRequestMutex.lock();
    g_imageRequestConditions.insert(id, new QWaitCondition);
    QCoreApplication::postEvent(this, new QSpotifyRequestImageEvent(id));
    g_imageRequestConditions[id]->wait(&g_imageRequestMutex);
    delete g_imageRequestConditions.take(id);

    QImage im = g_imageRequestImages.take(id);

    g_imageRequestMutex.unlock();

    return im;
}

static void callback_image_loaded(sp_image *image, void *)
{
    QCoreApplication::postEvent(QSpotifySession::instance(), new QSpotifyReceiveImageEvent(image));
}

void QSpotifySession::sendImageRequest(const QString &id)
{
    sp_link *link = sp_link_create_from_string(id.toUtf8().constData());
    sp_image *image = sp_image_create_from_link(m_sp_session, link);
    sp_link_release(link);

    g_imageRequestObject.insert(image, id);
    sp_image_add_load_callback(image, callback_image_loaded, 0);
}

void QSpotifySession::receiveImageResponse(sp_image *image)
{
    sp_image_remove_load_callback(image, callback_image_loaded, 0);

    QString id = g_imageRequestObject.take(image);
    QImage im;
    if (sp_image_error(image) == SP_ERROR_OK) {
        size_t dataSize;
        const void *data = sp_image_data(image, &dataSize);
        im = QImage::fromData(reinterpret_cast<const uchar *>(data), dataSize, "JPG");
    }
    else
    {
        qDebug() << "Failed to load image: " << sp_error_message(sp_image_error(image));
    }

    sp_image_release(image);

    g_imageRequestMutex.lock();
    g_imageRequestImages.insert(id, im);
    g_imageRequestConditions[id]->wakeAll();
    g_imageRequestMutex.unlock();
}

bool QSpotifySession::isOnline() const
{
    return m_networkConfManager->isOnline();
}

void QSpotifySession::onOnlineChanged()
{
    checkNetworkAccess();
}

void QSpotifySession::configurationChanged()
{
    checkNetworkAccess();
}

void QSpotifySession::checkNetworkAccess()
{
    if (!m_networkConfManager->isOnline()) {
        sp_session_set_connection_type(m_sp_session, SP_CONNECTION_TYPE_NONE);
        setOfflineMode(true, true);
    } else {

        // TODO: fix this (roaming detection is wrong, but isn't currently used)

        bool wifi = false;
        bool mobile = false;
        bool roaming = false;
        QList<QNetworkConfiguration> confs = m_networkConfManager->allConfigurations(QNetworkConfiguration::Active);
        for (int i = 0; i < confs.count(); ++i) {
            QString bearer = confs.at(i).bearerName();
            if (bearer == QLatin1String("WLAN")) {
                wifi = true;
                break;
            }
            if (bearer == QLatin1String("2G")
                    || bearer == QLatin1String("CDMA2000")
                    || bearer == QLatin1String("WCDMA")
                    || bearer == QLatin1String("HSPA")
                    || bearer == QLatin1String("WiMAX")) {
                mobile = true;
            }
            if (confs.at(i).isRoamingAvailable())
                roaming = true;
        }

        sp_connection_type type;
        if (wifi)
            type = SP_CONNECTION_TYPE_WIFI;
        else if (roaming)
            type = SP_CONNECTION_TYPE_MOBILE_ROAMING;
        else if (mobile)
            type = SP_CONNECTION_TYPE_MOBILE;
        else
            type = SP_CONNECTION_TYPE_UNKNOWN;

        sp_session_set_connection_type(m_sp_session, type);

        if (m_forcedOfflineMode)
            setOfflineMode(false, true);
        else
            setConnectionRule(AllowNetwork, !m_offlineMode);
    }
}

void QSpotifySession::setConnectionRules(ConnectionRules r)
{
    if (m_connectionRules == r)
        return;

    m_connectionRules = r;
    sp_session_set_connection_rules(m_sp_session, sp_connection_rules(int(m_connectionRules)));
    emit connectionRulesChanged();
}

void QSpotifySession::setConnectionRule(ConnectionRule r, bool on)
{
    ConnectionRules oldRules = m_connectionRules;
    if (on)
        m_connectionRules |= r;
    else
        m_connectionRules &= ~r;

    if (m_connectionRules != oldRules) {
        sp_session_set_connection_rules(m_sp_session, sp_connection_rules(int(m_connectionRules)));
        emit connectionRulesChanged();
    }
}

void QSpotifySession::setOfflineMode(bool on, bool forced)
{
    if (m_offlineMode == on)
        return;

    m_offlineMode = on;

    if (m_offlineMode && m_currentTrack && !m_currentTrack->isAvailableOffline())
        stop();

    if (m_offlineMode && m_radioMode)
        setRadioMode(false);

    if (!forced) {
        QSettings s;
        s.setValue(QLatin1String("offlineMode"), m_offlineMode);

        if (m_offlineMode)
            m_ignoreNextConnectionError = true;
    }

    m_forcedOfflineMode = forced && on;

    if (!m_offlineMode && !forced)
    {
        checkNetworkAccess();
    }

    setConnectionRule(AllowNetwork, !on);

    emit offlineModeChanged();
}

void QSpotifySession::setSyncOverMobile(bool s)
{
    if (m_syncOverMobile == s)
        return;

    m_syncOverMobile = s;

    QSettings settings;
    settings.setValue(QLatin1String("syncOverMobile"), m_syncOverMobile);

    setConnectionRule(AllowSyncOverMobile, s);
    emit syncOverMobileChanged();
}

void QSpotifySession::userLoaded()
{
    disconnect(m_user, SIGNAL(isLoadedChanged()), this, SLOT(userLoaded()));

    if (m_user->playlistContainer()->isLoaded())
    {
        playlistContainerLoaded();
    }
    else
    {
        connect(m_user->playlistContainer(), SIGNAL(isLoadedChanged()), this, SLOT(playlistContainerLoaded()));
    }
}

void QSpotifySession::playlistContainerLoaded()
{
    disconnect(m_user->playlistContainer(), SIGNAL(isLoadedChanged()), this, SLOT(playlistContainerLoaded()));


    m_playlistsIndex = 0;
    if (m_user->starredList()->isLoaded())
    {
        playlistLoaded();
    }
    else
    {
        connect(m_user->starredList(), SIGNAL(isLoadedChanged()), this, SLOT(playlistLoaded()));
    }
}

void QSpotifySession::playlistLoaded()
{
    QSpotifyPlaylist *playlist;

    if (m_playlistsIndex == 0)
    {
        playlist = m_user->starredList();
    }
    else
    {
        playlist = m_user->playlists()[m_playlistsIndex-1];
    }

    disconnect(playlist, SIGNAL(isLoadedChanged()), this, SLOT(playlistLoaded()));

    m_tracksIndex = 0;
    if (playlist->tracks().count() > 0)
    {
        if (playlist->tracks()[m_tracksIndex]->isLoaded())
        {
            trackLoaded();
        }
        else
        {
            connect(playlist->tracks()[m_tracksIndex], SIGNAL(isLoadedChanged()), this, SLOT(trackLoaded()));
        }
    }
    else
    {
        ++m_playlistsIndex;
        if (m_playlistsIndex - 1 < m_user->playlists().count())
        {
            playlist = m_user->playlists()[m_playlistsIndex - 1];

            if (playlist->isLoaded() == false)
            {
                connect(playlist, SIGNAL(isLoadedChanged()), this, SLOT(playlistLoaded()));
            }
            else
            {
                playlistLoaded();
            }
        }
    }
}


void QSpotifySession::trackLoaded()
{
    QSpotifyTrack *track;
    QSpotifyPlaylist *playlist;

    if (m_playlistsIndex == 0)
    {
        playlist = m_user->starredList();
    }
    else
    {
        playlist = m_user->playlists()[m_playlistsIndex-1];
    }
    track = playlist->tracks()[m_tracksIndex];

    disconnect(track, SIGNAL(isLoadedChanged()), this, SLOT(trackLoaded()));

    updateLastArtists(track->artistObject());

    if (m_lastArtists.count() < MAX_LAST_ARTISTS)
    {
        ++m_tracksIndex;
        if (m_tracksIndex < playlist->tracks().count())
        {
            // Move to next track in playlist
            if (playlist->tracks()[m_tracksIndex]->isLoaded())
            {
                trackLoaded();
            }
            else
            {
                connect(playlist->tracks()[m_tracksIndex], SIGNAL(isLoadedChanged()), this, SLOT(trackLoaded()));
            }
        }
        else if (m_playlistsIndex < m_user->playlists().count())
        {
            // Move to next playlist
            m_playlistsIndex++;

            playlist = m_user->playlists()[m_playlistsIndex - 1];

            if (playlist->isLoaded() == false)
            {
                connect(playlist, SIGNAL(isLoadedChanged()), this, SLOT(playlistLoaded()));
            }
            else
            {
                playlistLoaded();
            }
        }
    }
}


void QSpotifySession::saveState()
{
    QSettings settings;

    if (m_radioMode)
    {
        settings.setValue(QLatin1String("Radio Mode"), true);

        settings.setValue(QLatin1String("Radio Artist"), m_radio->getArtist().asLink());
    }
    else
    {
       settings.setValue(QLatin1String("Radio Mode"), false);

       // Get the playlist from the current track
       if (m_currentTrack && m_currentTrack->m_playlist)
       {
           settings.setValue(QLatin1String("Playlist"), m_currentTrack->m_playlist->asLink());
           settings.setValue(QLatin1String("Playlist Index"), m_playQueue->currentIndex());
       }
       else
       {
           settings.setValue(QLatin1String("Playlist"), QLatin1String(""));
       }
    }
}

void QSpotifySession::restoreState()
{
    QSettings settings;

    if (settings.value(QLatin1String("Radio Mode"), false) == true)
    {
        QString artist_link = settings.value(QLatin1String("Radio Artist"), QLatin1String("")).toString();

        if (artist_link.size() > 0)
        {
            QSpotifyArtist *artist = QSpotifyArtist::fromLink(artist_link);

            if (artist)
            {
                startRadio(artist);
            }
        }
    }
    else
    {
        QString playlist_link = settings.value(QLatin1String("Playlist"), QLatin1String("")).toString();

        if (playlist_link.size() > 0)
        {
            QSpotifyPlaylist *playlist = QSpotifyPlaylist::fromLink(playlist_link);
            int index = settings.value(QLatin1String("Playlist Index"), 0).toInt();

            if (playlist && index < playlist->trackCount())
            {
                static_cast<QSpotifyTrack*>(playlist->tracksAsQObject()[index])->play();
            }
            else if (playlist)
            {
                delete(playlist);
            }
        }
    }
}

void QSpotifySession::setKeepResources(bool keep)
{
    if (keep == m_keepResources)
        return;

    m_keepResources = keep;

    if (m_keepResources == false && !m_isPlaying)
    {
        m_resourceSet->release();
    }
    else if (m_keepResources == true && !m_resourceSet->hasResourcesGranted())
    {
        m_resourceSet->acquire();
    }

    emit keepResourcesChanged();
}

QString QSpotifySession::getErrorMessage(ConnectionError error)
{
    switch (error)
    {
    case ClientTooOld:
        //% "QTN_ERR_CLIENT_TOO_OLD"
        return qtTrId("QTN_ERR_CLIENT_TOO_OLD");

    case NetworkDisabled: // Show to user as "Unable to connect"
    case UnableToContactServer:
        //% "QTN_ERR_CANNOT_CONTACT_SERVER"
        return qtTrId("QTN_ERR_CANNOT_CONTACT_SERVER");

    case BadUsernameOrPassword:
        //% "QTN_ERR_USER_OR_PASSWORD"
        return qtTrId("QTN_ERR_USER_OR_PASSWORD");

    case UserBanned:
        //% "QTN_ERR_USER_BANNED"
        return qtTrId("QTN_ERR_USER_BANNED");

    case UserNeedsPremium:
        //% "QTN_ERR_USER_NEEDS_PREMIUM"
        return qtTrId("QTN_ERR_USER_NEEDS_PREMIUM");

    case RadioError:
        //% "QTN_ERR_RADIO_ERROR"
        return qtTrId("QTN_ERR_RADIO_ERROR");

    default:
        //% "QTN_ERR_UNKNOWN"
        return qtTrId("QTN_ERR_UNKNOWN");
    }
}

QString QSpotifySession::getErrorMessage(OfflineError error)
{
    switch(error)
    {
    case TooManyTracks:
        //% "QTN_ERR_TOO_MANY_OFFLINE_TRACKS"
        return qtTrId("QTN_ERR_TOO_MANY_OFFLINE_TRACKS");

    case DiskCache:
        //% "QTN_ERR_DISK_CACHE_FULL"
        return qtTrId("QTN_ERR_DISK_CACHE_FULL");

    case Expired:
        //% "QTN_ERR_OFFLINE_EXPIRED"
        return qtTrId("QTN_ERR_OFFLINE_EXPIRED");

    case NotAllowed:
        //% "QTN_ERR_OFFLINE_NOT_ALLOWED"
        return qtTrId("QTN_ERR_OFFLINE_NOT_ALLOWED");

    case LicenseLost:
        //% "QTN_ERR_OFFLINE_LICENSE_LOST"
        return qtTrId("QTN_ERR_OFFLINE_LICENSE_LOST");

    case LicenseError:
        //% "QTN_ERR_LICENSE_SERVER_ERROR"
        return qtTrId("QTN_ERR_LICENSE_SERVER_ERROR");

    default:
        //% "QTN_ERR_UNKNOWN"
        return qtTrId("QTN_ERR_UNKNOWN");
    }
}
