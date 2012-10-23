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

#include "qspotifyplaylist.h"
#include "qspotifytrack.h"
#include "qspotifysession.h"
#include "qspotifyplayqueue.h"
#include "qspotifyuser.h"
#include "qspotifyalbumbrowse.h"
#include "qspotifyutils.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QHash>
#include <QtCore/QDebug>

static QHash<sp_playlist*, QSpotifyPlaylist*> g_playlistObjects;

class QSpotifyTracksAddedEvent : public QEvent
{
public:
    QSpotifyTracksAddedEvent(QVector<sp_track*> tracks, int pos)
        : QEvent(Type(User + 3))
        , m_tracks(tracks)
        , m_position(pos)
    { }

    QVector<sp_track*> tracks() const { return m_tracks; }
    int position() const { return m_position; }

private:
    QVector<sp_track*> m_tracks;
    int m_position;
};

class QSpotifyTracksRemovedEvent : public QEvent
{
public:
    QSpotifyTracksRemovedEvent(QVector<int> positions)
        : QEvent(Type(User + 4))
        , m_positions(positions)
    { }

    QVector<int> positions() const { return m_positions; }

private:
    QVector<int> m_positions;
};

class QSpotifyTracksMovedEvent : public QEvent
{
public:
    QSpotifyTracksMovedEvent(QVector<int> positions, int newpos)
        : QEvent(Type(User + 5))
        , m_positions(positions)
        , m_newposition(newpos)
    { }

    QVector<int> positions() const { return m_positions; }
    int newPosition() const { return m_newposition; }

private:
    QVector<int> m_positions;
    int m_newposition;
};

class QSpotifyTrackSeenEvent : public QEvent
{
public:
    QSpotifyTrackSeenEvent(int pos, bool seen)
        : QEvent(Type(User + 6))
        , m_position(pos)
        , m_seen(seen)
    { }

    int position() const { return m_position; }
    bool seen() const { return m_seen; }

private:
    int m_position;
    bool m_seen;
};

static void callback_playlist_state_changed(sp_playlist *playlist, void *)
{
    QCoreApplication::postEvent(g_playlistObjects.value(playlist), new QEvent(QEvent::User));
}

static void callback_playlist_metadata_updated(sp_playlist *playlist, void *)
{
    QCoreApplication::postEvent(g_playlistObjects.value(playlist), new QEvent(QEvent::Type(QEvent::User + 1)));
}

static void callback_playlist_renamed(sp_playlist *playlist, void *)
{
    QCoreApplication::postEvent(g_playlistObjects.value(playlist), new QEvent(QEvent::Type(QEvent::User + 2)));
}

static void callback_tracks_added(sp_playlist *pl, sp_track *const *tracks, int num_tracks, int position, void *)
{
    QVector<sp_track*> vec;
    for (int i = 0; i < num_tracks; ++i)
        vec.append(tracks[i]);
    QCoreApplication::postEvent(g_playlistObjects.value(pl), new QSpotifyTracksAddedEvent(vec, position));
}

static void callback_tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks, void *)
{
    QVector<int> vec;
    for (int i = 0; i < num_tracks; ++i)
        vec.append(tracks[i]);
    QCoreApplication::postEvent(g_playlistObjects.value(pl), new QSpotifyTracksRemovedEvent(vec));
}

static void callback_tracks_moved(sp_playlist *pl, const int *tracks, int num_tracks, int new_position, void *)
{
    QVector<int> vec;
    for (int i = 0; i < num_tracks; ++i)
        vec.append(tracks[i]);
    QCoreApplication::postEvent(g_playlistObjects.value(pl), new QSpotifyTracksMovedEvent(vec, new_position));
}

static void callback_track_seen_changed(sp_playlist *pl, int position, bool seen, void *)
{
    QCoreApplication::postEvent(g_playlistObjects.value(pl), new QSpotifyTrackSeenEvent(position, seen));
}


QSpotifyPlaylist::QSpotifyPlaylist(Type type, sp_playlist *playlist, bool incrRefCount)
    : QSpotifyObject(true)
    , m_type(type)
    , m_offlineStatus(No)
    , m_collaborative(false)
    , m_offlineDownloadProgress(0)
    , m_availableOffline(false)
    , m_skipUpdateTracks(false)
{
    m_trackList = new QSpotifyTrackList(type == Starred || type == Inbox);

    if (incrRefCount)
        sp_playlist_add_ref(playlist);
    m_sp_playlist = playlist;
    g_playlistObjects.insert(playlist, this);
    m_callbacks = new sp_playlist_callbacks;
    m_callbacks->playlist_state_changed = callback_playlist_state_changed;
    m_callbacks->description_changed = 0;
    m_callbacks->image_changed = 0;
    m_callbacks->playlist_metadata_updated = callback_playlist_metadata_updated;
    m_callbacks->playlist_renamed = callback_playlist_renamed;
    m_callbacks->playlist_update_in_progress = 0;
    m_callbacks->subscribers_changed = 0;
    m_callbacks->tracks_added = callback_tracks_added;
    m_callbacks->tracks_moved = callback_tracks_moved;
    m_callbacks->tracks_removed = callback_tracks_removed;
    m_callbacks->track_created_changed = 0;
    m_callbacks->track_message_changed = 0;
    m_callbacks->track_seen_changed = callback_track_seen_changed;
    sp_playlist_add_callbacks(m_sp_playlist, m_callbacks, 0);
    connect(this, SIGNAL(dataChanged()), this, SIGNAL(playlistDataChanged()));
    connect(this, SIGNAL(isLoadedChanged()), this, SIGNAL(thisIsLoadedChanged()));
    connect(this, SIGNAL(playlistDataChanged()), this , SIGNAL(seenCountChanged()));
    connect(this, SIGNAL(playlistDataChanged()), this, SIGNAL(tracksChanged()));

    metadataUpdated();
}

QSpotifyPlaylist::~QSpotifyPlaylist()
{
    emit playlistDestroyed();
    g_playlistObjects.remove(m_sp_playlist);
    sp_playlist_remove_callbacks(m_sp_playlist, m_callbacks, 0);
    m_trackList->release();
    sp_playlist_release(m_sp_playlist);
    delete m_callbacks;
}

bool QSpotifyPlaylist::isLoaded()
{
    return sp_playlist_is_loaded(m_sp_playlist) && (m_trackList->tracks().count() == sp_playlist_num_tracks(m_sp_playlist));
}

bool QSpotifyPlaylist::updateData()
{
    bool updated = false;

    QString name = QString::fromUtf8(sp_playlist_name(m_sp_playlist));
    if (m_name != name) {
        m_name = name;
        updated = true;
    }

    QString owner = QString::fromUtf8(sp_user_canonical_name(sp_playlist_owner(m_sp_playlist)));
    if (m_owner != owner) {
        m_owner = owner;
        updated = true;
    }

    bool collab = sp_playlist_is_collaborative(m_sp_playlist);
    if (m_collaborative != collab) {
        m_collaborative = collab;
        updated = true;
    }

    if (m_trackList->m_tracks.isEmpty() && !m_skipUpdateTracks) {
        int count = sp_playlist_num_tracks(m_sp_playlist);
        QVector<sp_track*> updateTracks(count);
        for (int i = 0; i < count; ++i)
            updateTracks[i] = sp_playlist_track(m_sp_playlist, i);
        addTracks(updateTracks);
        updated = true;
    }

    OfflineStatus os = OfflineStatus(sp_playlist_get_offline_status(QSpotifySession::instance()->spsession(), m_sp_playlist));
    if (m_offlineStatus != os) {
        if (os == Waiting && m_offlineTracks.count() == m_availableTracks.count())
            m_offlineStatus = Yes;
        else if (os == Yes && m_offlineTracks.count() < m_availableTracks.count())
            m_offlineStatus = Waiting;
        else
            m_offlineStatus = os;

        if (m_offlineStatus != No) {
            m_availableOffline = true;
            emit availableOfflineChanged();
        }

        emit offlineStatusChanged();

        updated = true;
    }

    if (m_offlineStatus == Downloading) {
        int dp = sp_playlist_get_offline_download_completed(QSpotifySession::instance()->spsession(), m_sp_playlist);
        if (m_offlineDownloadProgress != dp) {
            m_offlineDownloadProgress = dp;
            updated = true;
        }
    }

    return updated;
}

void QSpotifyPlaylist::addTracks(QVector<sp_track*> & tracks, int pos)
{
    QVector<QSpotifyTrack*> qTracks(tracks.count());

    for (int i = 0; i < tracks.count(); i++)
    {
        qTracks[i] = new QSpotifyTrack(tracks[i], this);

        registerTrackType(qTracks[i]);

        m_tracksSet.insert(tracks[i]);
        connect(qTracks[i], SIGNAL(trackDataChanged()), this, SIGNAL(playlistDataChanged()));
        connect(qTracks[i], SIGNAL(offlineStatusChanged()), this, SLOT(onTrackChanged()));
        connect(qTracks[i], SIGNAL(isAvailableChanged()), this, SLOT(onTrackChanged()));
        if (m_type != Starred) {
            connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksAdded(QVector<sp_track*>)), qTracks[i], SLOT(onStarredListTracksAdded(QVector<sp_track*>)));
            connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksRemoved(QVector<sp_track*>)), qTracks[i], SLOT(onStarredListTracksRemoved(QVector<sp_track*>)));
        }
        if (m_type == Inbox) {
            connect(qTracks[i], SIGNAL(seenChanged()), this, SIGNAL(seenCountChanged()));
        }
        qTracks[i]->metadataUpdated();
    }

    m_trackList->addTracks(qTracks, pos);
}

void QSpotifyPlaylist::removeTracks(QVector<int> & positions)
{
    QVector<sp_track *> tracksSignal;

    QVector<QSpotifyTrack*> tracks = m_trackList->removeTracks(positions);

    for (int i = 0; i < tracks.count(); ++i)
    {
        unregisterTrackType(tracks[i]);
        disconnect(tracks[i], SIGNAL(offlineStatusChanged()), this, SLOT(onTrackChanged()));
        disconnect(tracks[i], SIGNAL(isAvailableChanged()), this, SLOT(onTrackChanged()));
        tracksSignal.append(tracks[i]->m_sp_track);
        m_tracksSet.remove(tracks[i]->m_sp_track);
    }

    if (m_type == Starred)
        emit tracksRemoved(tracksSignal);

    for (int i = 0; i < tracks.count(); ++i)
    {
        tracks[i]->release();
    }
}

bool QSpotifyPlaylist::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        m_skipUpdateTracks = true;
        metadataUpdated();
        m_skipUpdateTracks = false;
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 1) {
        // TracksMetadata updated
        for (int i = 0; i < m_trackList->m_tracks.count(); ++i) {
            m_trackList->m_tracks.at(i)->metadataUpdated();
        }
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 2) {
        // Playlist renamed
        m_name = QString::fromUtf8(sp_playlist_name(m_sp_playlist));
        emit dataChanged();
        emit nameChanged();
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 3) {
        // TracksAdded event
        QSpotifyTracksAddedEvent *ev = static_cast<QSpotifyTracksAddedEvent *>(e);
        QVector<sp_track*> tracks = ev->tracks();
        addTracks(tracks, ev->position());
        emit dataChanged();
        if (m_type == Starred || m_type == Inbox)
            emit tracksAdded(tracks);
        if (QSpotifySession::instance()->playQueue()->isCurrentTrackList(m_trackList))
            QSpotifySession::instance()->playQueue()->tracksUpdated();
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 4) {
        // TracksRemoved event
        QSpotifyTracksRemovedEvent *ev = static_cast<QSpotifyTracksRemovedEvent *>(e);
        QVector<int> tracks = ev->positions();
        removeTracks(tracks);
        emit dataChanged();
        if (QSpotifySession::instance()->playQueue()->isCurrentTrackList(m_trackList))
            QSpotifySession::instance()->playQueue()->tracksUpdated();
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 5) {
        // TracksMoved event
        QSpotifyTracksMovedEvent *ev = static_cast<QSpotifyTracksMovedEvent *>(e);
        QVector<int> positions = ev->positions();
        int newpos = ev->newPosition();
        QVector<QSpotifyTrack*> tracks;
        for (int i = 0; i < positions.count(); ++i) {
            int pos = positions.at(i);
            if (pos < 0 || pos >= m_trackList->m_tracks.count())
                continue;
            tracks.append(m_trackList->m_tracks[pos]);
            m_trackList->m_tracks.replace(pos, 0);
        }
        for (int i = 0; i < tracks.count(); ++i)
            m_trackList->m_tracks.insert(newpos++, tracks.at(i));
        m_trackList->m_tracks.removeAll(0);
        emit dataChanged();
        if (QSpotifySession::instance()->playQueue()->isCurrentTrackList(m_trackList))
            QSpotifySession::instance()->playQueue()->tracksUpdated();
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 6) {
        // TrackSeen event
        if (m_type == Inbox) {
            QSpotifyTrackSeenEvent *ev = static_cast<QSpotifyTrackSeenEvent*>(e);
            m_trackList->m_tracks.at(ev->position())->updateSeen(ev->seen());
        }
        e->accept();
        return true;
    }
    return QSpotifyObject::event(e);
}

void QSpotifyPlaylist::add(QSpotifyTrack *track)
{
    if (!track)
        return;

    sp_playlist_add_tracks(m_sp_playlist, const_cast<sp_track* const*>(&track->m_sp_track), 1, m_trackList->m_tracks.count(), QSpotifySession::instance()->spsession());
}

void QSpotifyPlaylist::remove(QSpotifyTrack *track)
{
    if (!track)
        return;

    int i = m_trackList->m_tracks.indexOf(track);
    if (i > -1)
        sp_playlist_remove_tracks(m_sp_playlist, &i, 1);
}

void QSpotifyPlaylist::addAlbum(QSpotifyAlbumBrowse *album)
{
    if (!album || !album->m_albumTracks)
        return;

    int c = album->m_albumTracks->m_tracks.count();
    if (c < 1)
        return;

    const sp_track *tracks[c];
    for (int i = 0; i < c; ++i)
        tracks[i] = album->m_albumTracks->m_tracks.at(i)->sptrack();
    sp_playlist_add_tracks(m_sp_playlist, const_cast<sp_track* const*>(tracks), c, m_trackList->m_tracks.count(), QSpotifySession::instance()->spsession());
}

void QSpotifyPlaylist::rename(const QString &name)
{
    if (name.trimmed().isEmpty())
        return;

    QString n = name;
    if (n.size() > 255)
        n.resize(255);

    sp_playlist_rename(m_sp_playlist, n.toUtf8().constData());
}

int QSpotifyPlaylist::trackCount() const
{
    int c = 0;
    for (int i = 0; i < m_trackList->m_tracks.count(); ++i) {
        if (m_trackList->m_tracks.at(i)->error() == QSpotifyTrack::Ok &&
            !m_trackList->m_tracks.at(i)->isNotAvailableFromSpotify())
            ++c;
    }
    return c;
}

static bool stringContainsWord(const QString &string, const QString &word)
{
    if (word.isEmpty())
        return true;

    int index = string.indexOf(word, 0, Qt::CaseInsensitive);

    if (index == -1)
        return false;

    if (index == 0 || string.at(index - 1) == QLatin1Char(' '))
        return true;

    return false;
}

QList<QObject*> QSpotifyPlaylist::tracksAsQObject() const
{
    QList<QObject*> list;
    if (m_type == Starred || m_type == Inbox) {
        // Reverse order for StarredList to get the most recents first
        for (int i = m_trackList->m_tracks.count() - 1; i >= 0 ; --i) {
            QSpotifyTrack *t = m_trackList->m_tracks[i];
            if (t->error() == QSpotifyTrack::Ok &&
                !t->isNotAvailableFromSpotify() && (m_trackFilter.isEmpty()
                                                    || stringContainsWord(t->name(), m_trackFilter)
                                                    || stringContainsWord(t->artists(), m_trackFilter)
                                                    || stringContainsWord(t->album(), m_trackFilter)
                                                    || stringContainsWord(t->creator(), m_trackFilter))) {
                list.append((QObject*)(t));
            }
        }
    } else {
        for (int i = 0; i < m_trackList->m_tracks.count(); ++i) {
            QSpotifyTrack *t = m_trackList->m_tracks[i];
            if (t->error() == QSpotifyTrack::Ok &&
                !t->isNotAvailableFromSpotify() && (m_trackFilter.isEmpty()
                                                    || stringContainsWord(t->name(), m_trackFilter)
                                                    || stringContainsWord(t->artists(), m_trackFilter)
                                                    || stringContainsWord(t->album(), m_trackFilter))) {
                list.append((QObject*)(t));
            }
        }
    }
    return list;
}

int QSpotifyPlaylist::totalDuration() const
{
    return m_trackList->totalDuration();
}

QString QSpotifyPlaylist::listSection() const
{
    if (m_type == Playlist)
        return QLatin1String("p");
    else
        return QLatin1String("s");
}

void QSpotifyPlaylist::removeFromContainer()
{
    QSpotifySession::instance()->user()->removePlaylist(this);
}

bool QSpotifyPlaylist::isCurrentPlaylist() const
{
    return QSpotifySession::instance()->m_playQueue->m_implicitTracks == m_trackList;
}

void QSpotifyPlaylist::setCollaborative(bool c)
{
    sp_playlist_set_collaborative(m_sp_playlist, c);
}

void QSpotifyPlaylist::setAvailableOffline(bool offline)
{
    if (m_availableOffline == offline)
        return;

    m_availableOffline = offline;
    sp_playlist_set_offline_mode(QSpotifySession::instance()->spsession(), m_sp_playlist, offline);
    emit availableOfflineChanged();
}

void QSpotifyPlaylist::play()
{
    if (!m_trackList || m_trackList->m_tracks.isEmpty())
        return;

    if (QSpotifySession::instance()->shuffle())
        m_trackList->setShuffle(true);
    else
        m_trackList->setShuffle(false);

    int i = (m_type == Starred || m_type == Inbox) ? m_trackList->previousAvailable(m_trackList->m_tracks.count())
                                             : m_trackList->nextAvailable(-1);

    // If nothing is available we may have reached the end
    if (i >= m_trackList->m_tracks.count() || i < 0)
        return;

    if (QSpotifySession::instance()->m_radioMode)
    {
        QSpotifySession::instance()->setRadioMode(false);
    }

    QSpotifySession::instance()->m_playQueue->playTrack(m_trackList->m_tracks.at(i));
}

void QSpotifyPlaylist::enqueue()
{
    int c = m_trackList->m_tracks.count();
    if (m_type == Starred || m_type == Inbox) {
        // Reverse order for StarredList to get the most recents first
        QList<QSpotifyTrack *> tracks;
        for (int i = c - 1; i >= 0 ; --i)
           tracks.append(m_trackList->m_tracks.at(i));
        QSpotifySession::instance()->playQueue()->enqueueTracks(tracks);
    } else {
        QSpotifySession::instance()->playQueue()->enqueueTracks(m_trackList->m_tracks);
    }

    if (QSpotifySession::instance()->m_radioMode)
    {
        QSpotifySession::instance()->setRadioMode(false);
    }
}

int QSpotifyPlaylist::unseenCount() const
{
    if (m_type != Inbox)
        return 0;

    int c = 0;
    for (int i = 0; i < m_trackList->m_tracks.count(); ++i) {
        QSpotifyTrack *t = m_trackList->m_tracks.at(i);
        if (t->error() == QSpotifyTrack::Ok && !t->seen())
            ++c;
    }
    return c;
}

void QSpotifyPlaylist::onTrackChanged()
{
    if (!sender())
        return;

    QSpotifyTrack *tr = dynamic_cast<QSpotifyTrack *>(sender());
    if (!tr)
        return;

    registerTrackType(tr);
}

void QSpotifyPlaylist::registerTrackType(QSpotifyTrack *t)
{
    int oldCount = m_offlineTracks.count();
    if (t->offlineStatus() == QSpotifyTrack::Yes)
        m_offlineTracks.insert(t);
    else
        m_offlineTracks.remove(t);
    if ((oldCount == 0 && m_offlineTracks.count() > 0) || (oldCount == 1 && m_offlineTracks.count() == 0))
        emit hasOfflineTracksChanged();

    if (t->m_isAvailable) {
        m_availableTracks.insert(t);
    }
}

void QSpotifyPlaylist::unregisterTrackType(QSpotifyTrack *t)
{
    m_offlineTracks.remove(t);
    m_availableTracks.remove(t);
}

void QSpotifyPlaylist::setTrackFilter(const QString &filter)
{
    if (m_trackFilter == filter)
        return;

    m_trackFilter = filter;
    emit trackFilterChanged();
    emit tracksChanged();
}

QString QSpotifyPlaylist::asLink() const
{
    QString string;

    if (m_type == Playlist)
    {
        sp_link *playlist_link = sp_link_create_from_playlist(m_sp_playlist);

        string = QSpotifyUtils::splinkToString(*playlist_link);

        sp_link_release(playlist_link);
    }
    else
    {
        // For Starred / inbox we don't use sp_link, but simply have our own identifier
        if (m_type == Starred)
        {
            string = QLatin1String("QSpotifyPlaylist:Starred");
        }
        else
        {
            string = QLatin1String("QSpotifyPlaylist:Inbox");
        }
    }

    return string;
}

QSpotifyPlaylist *QSpotifyPlaylist::fromLink(const QString & link)
{
    QSpotifyPlaylist *playlist = 0;

    if (link == QLatin1String("QSpotifyPlaylist:Starred"))
    {
        playlist = QSpotifySession::instance()->m_user->starredList();
    }
    else if (link == QLatin1String("QSpotifyPlaylist:Inbox"))
    {
        playlist = QSpotifySession::instance()->m_user->inbox();
    }
    else
    {
        // Normal playlist

        sp_link *playlist_link = sp_link_create_from_string(link.toLocal8Bit().constData());
        sp_playlist *spplaylist = sp_playlist_create(QSpotifySession::instance()->m_sp_session, playlist_link);

        if (spplaylist)
        {
            playlist = new QSpotifyPlaylist(Playlist, spplaylist, false);
        }

        sp_link_release(playlist_link);
    }

    return playlist;
}
