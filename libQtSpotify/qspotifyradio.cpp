/****************************************************************************
**
** Copyright (c) 2012 Troels Nilsson.
** All rights reserved.
** Contact: nilsson.troels@gmail.com
**
** This file is part of the CarSpot project.
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
** Neither the name of CarSpot nor the names of its contributors may be
** used to endorse or promote products derived from this software without
** specific prior written permission.
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
****************************************************************************/

#include "qspotifyradio.h"

#include "qspotifyalbum.h"
#include "qspotifysession.h"
#include "qspotifyplayqueue.h"

QSpotifyRadio::QSpotifyRadio(QSpotifyArtist & originalArtist) :
    QObject(0)
  , m_readyForNextRadioSong(false)
  , m_playNextSong(false)
  , m_degreesOfSeperation(0)
{
    connect(&m_origArtistBrowser, SIGNAL(busyChanged()), this, SLOT(origBrowserBusyChanged()));
    m_origArtistBrowser.setArtist(&originalArtist);

    connect(&m_currentArtistBrowser, SIGNAL(busyChanged()), this, SLOT(currentBrowserBusyChanged()));

    connect(&m_albumBrowser, SIGNAL(busyChanged()), this, SLOT(albumBrowserBusyChanged()));
}

QSpotifyRadio::~QSpotifyRadio()
{
}

void QSpotifyRadio::origBrowserBusyChanged()
{
    if (!m_origArtistBrowser.busy() && !m_readyForNextRadioSong)
    {
        // Find the artist we want to use for the next song
        QList<QObject*> similarArtists = m_origArtistBrowser.similarArtists();

        if (similarArtists.isEmpty())
        {
            qDebug() << "No similar artists!";
            // We can't base a radio on this artist, raise an error
            emit errorOccured();
            return;
        }

        // Select a random similar artist or this artist
        int index = qrand() % (similarArtists.count() + 1);

        // index = similarArtists.count means we keep the same artist
        if (index != similarArtists.count())
        {
            QSpotifyArtist newArtist(dynamic_cast<QSpotifyArtist*>(similarArtists[index])->spartist());
            m_currentArtistBrowser.setArtist(&newArtist);
            m_degreesOfSeperation++;
        }
        else
        {
            m_currentArtistBrowser.setArtist(m_origArtistBrowser.artist());
        }
    }
}

void QSpotifyRadio::currentBrowserBusyChanged()
{
    if (!m_currentArtistBrowser.busy() && !m_readyForNextRadioSong)
    {
        QList<QObject*> albums = m_currentArtistBrowser.ownAlbums();

        if (albums.count() == 0)
        {
            qDebug() << "Artist doesn't have any albums!";
            findNextArtist();
            return;
        }

        // Select a random album
        int index = qrand() % albums.count();
        m_albumBrowser.setAlbum(dynamic_cast<QSpotifyAlbum*>(albums[index]));

        if (m_albumBrowser.busy() == false)
        {
            // No need to wait, proceed directly
            albumBrowserBusyChanged();
        }
    }
}

void QSpotifyRadio::albumBrowserBusyChanged()
{
    if (m_albumBrowser.busy() == false)
    {
        // Place the songs in the tracks list
        m_tracks.clear();

        m_tracks.append(m_albumBrowser.tracks());

        m_readyForNextRadioSong = true;

        if (m_playNextSong)
        {
            selectSong();
        }
    }
}

void QSpotifyRadio::nextSong()
{
    if (m_readyForNextRadioSong)
    {
        selectSong();
    }
    else
    {
        m_playNextSong = true;
        emit busyChanged();
    }
}

void QSpotifyRadio::selectSong()
{
    QList<QObject*> topTracks(m_currentArtistBrowser.topTracks());
    QList<QObject*> *tracks;
    int index;

    // 25% chance we select from the top tracks to give these extra weight
    if (qrand() % 4 == 0)
    {
        tracks = &topTracks;
    }
    else
    {
        // Select from the other tracks
        tracks = &m_tracks;
    }

    index = qrand() % tracks->count();

    QSpotifySession::instance()->m_playQueue->enqueueTrack(dynamic_cast<QSpotifyTrack*>((*tracks)[index]));
    QSpotifySession::instance()->m_playQueue->next();

    // Prepare to find the next song
    m_playNextSong = false;
    m_readyForNextRadioSong = false;
    emit busyChanged();

    findNextArtist();
}

void QSpotifyRadio::findNextArtist()
{
    QList<QObject*> similarArtists;
    int index;

    similarArtists = m_currentArtistBrowser.similarArtists();

    // Select a random similar artist or stay with this artist (index == similarArtists.count means
    // stay with the current artist)
    index = qrand() % (similarArtists.count() + 1);

    if (m_degreesOfSeperation >= MAX_DEGREES_OF_SEPERATION && index < similarArtists.count())
    {
        // Too far away from starting point, use the original artist
        similarArtists = m_origArtistBrowser.similarArtists();
        m_degreesOfSeperation = 0;
    }

    if (index != similarArtists.count())
    {
        QSpotifyArtist newArtist(dynamic_cast<QSpotifyArtist*>(similarArtists[index])->spartist());
        m_currentArtistBrowser.setArtist(&newArtist);
        m_degreesOfSeperation++;
    }
    else
    {
        // No need to wait for the current artist browser to be ready, so call the slot directly
        currentBrowserBusyChanged();
    }
}

const QSpotifyArtist & QSpotifyRadio::getArtist() const
{
    return *(m_origArtistBrowser.artist());
}
