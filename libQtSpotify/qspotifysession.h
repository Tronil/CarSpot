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

#ifndef QSPOTIFYSESSION_H
#define QSPOTIFYSESSION_H

#include <libspotify/api.h>
#include <policy/resource-set.h>

#include <QtCore/QObject>
#include <QtCore/QPair>
#include <QtGui/QImage>

class QSpotifyUser;
class QSpotifyTrack;
class QSpotifyAudioThread;
class QAudioOutput;
class QSpotifyPlayQueue;
class QNetworkConfigurationManager;
class QSpotifyArtist;
class QSpotifyArtistBrowse;
class QSpotifyRadio;
class QSpotifyPlaylistContainer;

class QSpotifySession : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ConnectionStatus connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(ConnectionError connectionError READ connectionError NOTIFY connectionErrorChanged)
    Q_PROPERTY(QString connectionErrorMessage READ connectionErrorMessage NOTIFY connectionErrorChanged)
    Q_PROPERTY(OfflineError offlineError READ offlineError NOTIFY offlineErrorChanged)
    Q_PROPERTY(QString offlineErrorMessage READ offlineErrorMessage NOTIFY offlineErrorChanged)
    Q_PROPERTY(bool pendingConnectionRequest READ pendingConnectionRequest NOTIFY pendingConnectionRequestChanged)
    Q_PROPERTY(QSpotifyUser *user READ user NOTIFY userChanged)
    Q_PROPERTY(QSpotifyTrack *currentTrack READ currentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(QSpotifyPlayQueue *playQueue READ playQueue NOTIFY userChanged)
    Q_PROPERTY(bool hasCurrentTrack READ hasCurrentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(int currentTrackPosition READ currentTrackPosition NOTIFY currentTrackPositionChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(bool shuffle READ shuffle WRITE setShuffle NOTIFY shuffleChanged)
    Q_PROPERTY(bool repeat READ repeat WRITE setRepeat NOTIFY repeatChanged)
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY isOnlineChanged)
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY isLoggedInChanged)
    Q_PROPERTY(bool offlineMode READ offlineMode NOTIFY offlineModeChanged)
    Q_PROPERTY(StreamingQuality streamingQuality READ streamingQuality WRITE setStreamingQuality NOTIFY streamingQualityChanged)
    Q_PROPERTY(StreamingQuality syncQuality READ syncQuality WRITE setSyncQuality NOTIFY syncQualityChanged)
    Q_PROPERTY(bool syncOverMobile READ syncOverMobile WRITE setSyncOverMobile NOTIFY syncOverMobileChanged)
    Q_PROPERTY(bool radioMode READ radioMode NOTIFY radioModeChanged)
    Q_PROPERTY(bool radioBusy READ radioBusy NOTIFY radioBusyChanged)
    Q_PROPERTY(QList<QObject *> lastArtists READ lastArtists NOTIFY lastArtistsChanged)
    Q_PROPERTY(bool keepResources READ keepResources WRITE setKeepResources NOTIFY keepResourcesChanged)
    Q_ENUMS(ConnectionStatus)
    Q_ENUMS(ConnectionError)
    Q_ENUMS(OfflineError)
    Q_ENUMS(StreamingQuality)
public:
    enum ConnectionStatus {
        LoggedOut = SP_CONNECTION_STATE_LOGGED_OUT,
        LoggedIn = SP_CONNECTION_STATE_LOGGED_IN,
        Disconnected = SP_CONNECTION_STATE_DISCONNECTED,
        Undefined = SP_CONNECTION_STATE_UNDEFINED,
        Offline = SP_CONNECTION_STATE_OFFLINE
    };

    enum ConnectionError {
        ConnectionOk = SP_ERROR_OK,
        ClientTooOld = SP_ERROR_CLIENT_TOO_OLD,
        UnableToContactServer = SP_ERROR_UNABLE_TO_CONTACT_SERVER,
        BadUsernameOrPassword = SP_ERROR_BAD_USERNAME_OR_PASSWORD,
        UserBanned = SP_ERROR_USER_BANNED,
        UserNeedsPremium = SP_ERROR_USER_NEEDS_PREMIUM,
        OtherTransient = SP_ERROR_OTHER_TRANSIENT,
        OtherPermanent = SP_ERROR_OTHER_PERMANENT,
        NetworkDisabled = SP_ERROR_NETWORK_DISABLED,
        RadioError = SP_ERROR_SYSTEM_FAILURE + 1
    };

    enum OfflineError {
        OfflineOk = SP_ERROR_OK,
        TooManyTracks = SP_ERROR_OFFLINE_TOO_MANY_TRACKS,
        DiskCache = SP_ERROR_OFFLINE_DISK_CACHE,
        Expired = SP_ERROR_OFFLINE_EXPIRED,
        NotAllowed = SP_ERROR_OFFLINE_NOT_ALLOWED,
        LicenseLost = SP_ERROR_OFFLINE_LICENSE_LOST,
        LicenseError = SP_ERROR_OFFLINE_LICENSE_ERROR
    };

    enum StreamingQuality {
        Unknown = -1,
        LowQuality = SP_BITRATE_96k,
        HighQuality = SP_BITRATE_160k
    };

    enum ConnectionRule {
        AllowNetwork = SP_CONNECTION_RULE_NETWORK,
        AllowNetworkIfRoaming = SP_CONNECTION_RULE_NETWORK_IF_ROAMING,
        AllowSyncOverMobile = SP_CONNECTION_RULE_ALLOW_SYNC_OVER_MOBILE,
        AllowSyncOverWifi = SP_CONNECTION_RULE_ALLOW_SYNC_OVER_WIFI
    };
    Q_DECLARE_FLAGS(ConnectionRules, ConnectionRule)

    enum Events {
        NotifyMainThreadEvent = QEvent::User,
        ConnectionErrorEvent,
        MetaDataUpdatedEvent,
        EndOfTrackEvent,
        AudioFormatNotSupportedEvent,
        TrackProgressEvent,
        RequestImageEvent,
        ReceiveImageEvent,
        PlayTokenLostEvent,
        LoggedInEvent,
        LoggedOutEvent,
        OfflineErrorEvent,
        ConnectionStateUpdatedEvent
    };

    ~QSpotifySession();

    static QSpotifySession *instance();

    Q_INVOKABLE bool isValid() const { return m_sp_session != 0; }

    Q_INVOKABLE QString formatDuration(qint64 d) const;

    ConnectionStatus connectionStatus() const { return m_connectionStatus; }
    void setConnectionStatus(ConnectionStatus status);

    bool isLoggedIn() const { return m_isLoggedIn; }

    bool offlineMode() const { return m_offlineMode; }
    Q_INVOKABLE void setOfflineMode(bool on, bool forced = false);

    ConnectionError connectionError() const { return m_connectionError; }
    void setConnectionError(ConnectionError error);

    QString connectionErrorMessage() const { return m_connectionErrorMessage; }

    OfflineError offlineError() const { return m_offlineError; }
    QString offlineErrorMessage() const { return m_offlineErrorMessage; }

    bool pendingConnectionRequest() const { return m_pending_connectionRequest; }

    QSpotifyUser *user() const { return m_user; }

    QSpotifyTrack *currentTrack() const { return m_currentTrack; }
    bool hasCurrentTrack() const { return m_currentTrack != 0; }
    int currentTrackPosition() const { return m_currentTrackPosition; }
    int currentTrackPlayedDuration() const { return m_currentTrackPlayedDuration; }

    StreamingQuality streamingQuality() const { return m_streamingQuality; }
    void setStreamingQuality(StreamingQuality q);

    StreamingQuality syncQuality() const { return m_syncQuality; }
    void setSyncQuality(StreamingQuality q);

    bool syncOverMobile() const { return m_syncOverMobile; }
    Q_INVOKABLE void setSyncOverMobile(bool s);

    bool isOnline() const;

    void play(QSpotifyTrack *track);
    bool setCurrentTrack(QSpotifyTrack *track);

    bool isPlaying() const { return m_isPlaying; }

    bool shuffle() const { return m_shuffle; }
    void setShuffle(bool s);

    bool repeat() const { return m_repeat; }
    void setRepeat(bool r);

    bool radioMode() const { return m_radioMode; }
    bool radioBusy() const;

    QList<QObject*> lastArtists() const;

    sp_session *spsession() const { return m_sp_session; }

    QSpotifyPlayQueue *playQueue() const { return m_playQueue; }

    bool keepResources() const { return m_keepResources; }
    void setKeepResources(bool keep);

public Q_SLOTS:
    void login(const QString &username, const QString &password = QString());
    void logout(bool keepLoginInfo);

    void pause();
    void resume();
    void stop(bool dontEmitSignals = false);
    void seek(int offset);
    void playNext();
    void playPrevious();
    void enqueue(QSpotifyTrack *track);
    void startRadio(QSpotifyArtist *artist); // Start radio like functionality based on the artist given

Q_SIGNALS:
    void connectionStatusChanged();
    void connectionErrorChanged();
    void offlineErrorChanged();
    void userChanged();
    void metadataUpdated();
    void currentTrackChanged();
    void pendingConnectionRequestChanged();
    void isPlayingChanged();
    void loggingIn();
    void loggingOut();
    void streamingQualityChanged();
    void syncQualityChanged();
    void currentTrackPositionChanged();
    void shuffleChanged();
    void repeatChanged();
    void isOnlineChanged();
    void playTokenLost();
    void connectionRulesChanged();
    void offlineModeChanged();
    void syncOverMobileChanged();
    void isLoggedInChanged();
    void radioModeChanged();
    void radioBusyChanged();
    void lastArtistsChanged();
    void keepResourcesChanged();

protected:
    bool event(QEvent *);

private Q_SLOTS:
    void resourceAcquiredHandler(const QList<ResourcePolicy::ResourceType>&);
    void resourceLostHandler();
    void resourceReleasedByManager();
    void onOnlineChanged();
    void configurationChanged();

    // For radio harvesting of artists
    void userLoaded();
    void playlistContainerLoaded();
    void playlistLoaded();
    void trackLoaded();

    void radioErrorOccurred();

private:
    QSpotifySession();
    void init();
    void cleanUp();
    void checkNetworkAccess();
    void processSpotifyEvents();
    void beginPlayBack();

    void onLoggedIn();
    void onLoggedOut();

    QString getStoredLoginInformation() const;

    QImage requestSpotifyImage(const QString &id);
    void sendImageRequest(const QString &id);
    void receiveImageResponse(sp_image *image);

    void setConnectionRules(ConnectionRules r);
    void setConnectionRule(ConnectionRule r, bool on = true);

    void setRadioMode(bool mode);

    void updateLastArtists(QSpotifyArtist *new_artist);

    void saveState();
    void restoreState();

    QString getErrorMessage(ConnectionError error);
    QString getErrorMessage(OfflineError error);

    static QSpotifySession *m_instance;
    int m_timerID;

    sp_session *m_sp_session;
    sp_session_callbacks m_sp_callbacks;
    sp_session_config m_sp_config;

    ConnectionStatus m_connectionStatus;
    ConnectionError m_connectionError;
    OfflineError m_offlineError;
    ConnectionRules m_connectionRules;
    QString m_connectionErrorMessage;
    QString m_offlineErrorMessage;
    StreamingQuality m_streamingQuality;
    StreamingQuality m_syncQuality;
    bool m_syncOverMobile;

    QSpotifyUser *m_user;

    bool m_pending_connectionRequest;
    bool m_isLoggedIn;
    bool m_explicitLogout;

    bool m_offlineMode;
    bool m_forcedOfflineMode;
    bool m_ignoreNextConnectionError;

    QSpotifyPlayQueue *m_playQueue;
    QSpotifyTrack *m_currentTrack;
    bool m_isPlaying;
    int m_currentTrackPosition;
    int m_currentTrackPlayedDuration;
    bool m_shuffle;
    bool m_repeat;

    bool m_radioMode;
    QSpotifyRadio *m_radio;

    QList<QSpotifyArtist *> m_lastArtists;
    enum { MAX_LAST_ARTISTS = 10 };
    int m_playlistsIndex;
    int m_tracksIndex;

    QSpotifyAudioThread *m_audioThread;

    // Phone Resource Management
    ResourcePolicy::ResourceSet *m_resourceSet;
    ResourcePolicy::AudioResource *m_audioResource;
    ResourcePolicy::ScaleButtonResource *m_scaleButtonResource;

    bool m_keepResources;

    // Network Management
    QNetworkConfigurationManager *m_networkConfManager;

    friend class QSpotifyUser;
    friend class QSpotifyTrack;
    friend class QSpotifyImageProvider;
    friend class QSpotifySearch;
    friend class QSpotifyPlaylist;
    friend class QSpotifyRadio;
    friend class QSpotifyPlayQueue;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSpotifySession::ConnectionRules)

#endif // QSPOTIFYSESSION_H
