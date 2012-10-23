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

#include "qspotifyartistbrowse.h"

#include "qspotifyartist.h"
#include "qspotifyalbum.h"
#include "qspotifysession.h"
#include "qspotifytracklist.h"
#include "qspotifytrack.h"
#include "qspotifyuser.h"
#include "qspotifyplaylist.h"

#include <QtCore/QHash>
#include <QtConcurrentRun>
#include <libspotify/api.h>

static QHash<sp_artistbrowse*, QSpotifyArtistBrowse*> g_artistBrowseObjects;
static QMutex g_mutex;

static void callback_artistbrowse_complete(sp_artistbrowse *result, void *)
{
    QMutexLocker lock(&g_mutex);
    QSpotifyArtistBrowse *o = g_artistBrowseObjects.value(result);
    if (o)
        QCoreApplication::postEvent(o, new QEvent(QEvent::User));
}

QSpotifyArtistBrowse::QSpotifyArtistBrowse(QObject *parent)
    : QObject(parent)
    , m_sp_artistbrowse(0)
    , m_artist(0)
    , m_topTracks(0)
    , m_busy(false)
    , m_dataReady(false)
{
}

QSpotifyArtistBrowse::~QSpotifyArtistBrowse()
{
    clearData();
}

void QSpotifyArtistBrowse::setArtist(QSpotifyArtist *artist)
{
    if (m_artist == artist)
        return;
    clearData();
    m_artist = new QSpotifyArtist(artist->spartist());
    emit artistChanged();

    if (!m_artist)
        return;

    m_dataReady = false;
    m_busy = true;
    emit busyChanged();

    QMutexLocker lock(&g_mutex);
    m_sp_artistbrowse = sp_artistbrowse_create(QSpotifySession::instance()->spsession(),
                                               m_artist->spartist(),
                                               SP_ARTISTBROWSE_NO_TRACKS,
                                               callback_artistbrowse_complete, 0);
    g_artistBrowseObjects.insert(m_sp_artistbrowse, this);
}

QList<QObject *> QSpotifyArtistBrowse::topTracks() const
{
    QList<QObject*> list;
    if (m_topTracks != 0) {
        int c = m_topTracks->m_tracks.count();
        for (int i = 0; i < c; ++i)
            list.append((QObject*)(m_topTracks->m_tracks[i]));
    }
    return list;
}

bool QSpotifyArtistBrowse::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        g_mutex.lock();
        g_artistBrowseObjects.remove(m_sp_artistbrowse);
        g_mutex.unlock();
        processData();
        e->accept();
        return true;
    }
    return QObject::event(e);
}

void QSpotifyArtistBrowse::clearData()
{
    if (m_sp_artistbrowse) {
        QMutexLocker lock(&g_mutex);
        g_artistBrowseObjects.remove(m_sp_artistbrowse);
        sp_artistbrowse_release(m_sp_artistbrowse);
        m_sp_artistbrowse = 0;
    }
    m_biography.clear();
    if (m_topTracks)
        m_topTracks->release();
    m_topTracks = 0;
    qDeleteAll(m_albums);
    m_albums.clear();
    qDeleteAll(m_singles);
    m_singles.clear();
    qDeleteAll(m_compilations);
    m_compilations.clear();
    qDeleteAll(m_appearsOn);
    m_appearsOn.clear();
    qDeleteAll(m_similarArtists);
    m_similarArtists.clear();
    delete m_artist;
    m_artist = 0;
}

void QSpotifyArtistBrowse::processData()
{
    if (m_sp_artistbrowse) {
        m_dataReady = true;

        if (sp_artistbrowse_error(m_sp_artistbrowse) != SP_ERROR_OK)
            return;

        m_biography = QString::fromUtf8(sp_artistbrowse_biography(m_sp_artistbrowse)).split(QLatin1Char('\n'), QString::SkipEmptyParts);

        if (sp_artistbrowse_num_portraits(m_sp_artistbrowse) > 0) {
            sp_link *link = sp_link_create_from_artistbrowse_portrait(m_sp_artistbrowse, 0);
            char buffer[200];
            int uriSize = sp_link_as_string(link, &buffer[0], 200);
            m_pictureId = QString::fromUtf8(&buffer[0], uriSize);
            sp_link_release(link);
        }

        int c = qMin(80, sp_artistbrowse_num_albums(m_sp_artistbrowse));
        for (int i = 0; i < c; ++i) {
            sp_album *album = sp_artistbrowse_album(m_sp_artistbrowse, i);
            if (!sp_album_is_available(album))
                continue;
            QSpotifyAlbum *qalbum = new QSpotifyAlbum(album);
            if ((qalbum->type() == QSpotifyAlbum::Album || qalbum->type() == QSpotifyAlbum::Unknown) && qalbum->artist() == m_artist->name()) {
                m_albums.append((QObject *)qalbum);
            } else if (qalbum->type() == QSpotifyAlbum::Single) {
                m_singles.append((QObject *)qalbum);
            } else if (qalbum->type() == QSpotifyAlbum::Compilation) {
                m_compilations.append((QObject *)qalbum);
            } else {
                m_appearsOn.append((QObject *)qalbum);
            }
        }

        QStringList similArt;
        c = sp_artistbrowse_num_similar_artists(m_sp_artistbrowse);
        for (int i = 0; i < c; ++i) {
            QSpotifyArtist *artist = new QSpotifyArtist(sp_artistbrowse_similar_artist(m_sp_artistbrowse, i));
            m_similarArtists.append((QObject *)artist);
            similArt.append(QString(QLatin1String("\"%1\"")).arg(artist->name()));
        }

        processTophits();

        m_busy = false;
        emit dataChanged();
        emit busyChanged();
    }
}

void QSpotifyArtistBrowse::processTophits()
{
    m_topTracks = new QSpotifyTrackList;
    int c = sp_artistbrowse_num_tophit_tracks(m_sp_artistbrowse);
    for (int i = 0; i < c; ++i) {
        QSpotifyTrack *t =  new QSpotifyTrack(sp_artistbrowse_tophit_track(m_sp_artistbrowse, i), m_topTracks);
        m_topTracks->m_tracks.append(t);
    }
}
