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

#include "qspotifytracklist.h"

#include "qspotifytrack.h"
#include "qspotifysession.h"

QSpotifyTrackList::QSpotifyTrackList(bool reverse)
    : QObject()
    , m_reverse(reverse)
    , m_currentIndex(0)
    , m_currentTrack(0)
    , m_shuffle(false)
    , m_shuffleList(0)
    , m_shuffleIndex(0)
    , m_refCount(1)
    , m_startPlayback(false)
{
}

QSpotifyTrackList::~QSpotifyTrackList()
{
    int c = m_tracks.count();
    for (int i = 0; i < c; ++i)
        m_tracks[i]->release();
}

void QSpotifyTrackList::setCurrentTrackToFirst()
{
    if (m_tracks.count() == 0)
        return;

    setCurrentTrackTo(m_reverse ? previousAvailable(m_tracks.count()) : nextAvailable(-1));
}

void QSpotifyTrackList::play()
{
    m_startPlayback = true;
    setCurrentTrackToFirst();
}

bool QSpotifyTrackList::playTrackAtIndex(int i)
{
    m_startPlayback = true;
    return setCurrentTrackTo(i);
}

bool QSpotifyTrackList::setCurrentTrackTo(int i)
{
    if (i < 0 || i >= m_tracks.count())
    {
        if (m_currentTrack)
        {
            m_currentTrack->release();
        }

        m_currentTrack = 0;
        m_currentIndex = 0;

        return false;
    }

    if (m_shuffle)
        m_shuffleIndex = m_shuffleList.indexOf(i);
    m_currentTrack = m_tracks.at(i);
    m_currentTrack->addRef();
    m_currentIndex = i;

    setCurrentTrack();

    return true;
}

bool QSpotifyTrackList::next()
{
    int index;

    if (m_shuffle)
    {
        index = m_shuffleList[m_shuffleIndex];
    }
    else
    {
        index = m_currentIndex;
    }

    return setCurrentTrackTo(m_reverse ? previousAvailable(index) : nextAvailable(index));
}

bool QSpotifyTrackList::previous()
{
    int index;

    if (m_shuffle)
    {
        index = m_shuffleList[m_shuffleIndex];
    }
    else
    {
        index = m_currentIndex;
    }

    return setCurrentTrackTo(m_reverse ? nextAvailable(index) : previousAvailable(index));
}

void QSpotifyTrackList::setCurrentTrackToLast()
{
    if (m_tracks.count() == 0)
        return;

    setCurrentTrackTo(m_reverse ? nextAvailable(-1) : previousAvailable(m_tracks.count()));
}

void QSpotifyTrackList::setCurrentTrack()
{
    if (!m_currentTrack)
        return;

    if (m_currentTrack->isLoaded())
        onTrackReady();
    else
        connect(m_currentTrack, SIGNAL(isLoadedChanged()), this, SLOT(onTrackReady()));
}

void QSpotifyTrackList::onTrackReady()
{
    disconnect(this, SLOT(onTrackReady()));
    if (QSpotifySession::instance()->isPlaying() || m_startPlayback)
    {
        m_startPlayback = false;
        QSpotifySession::instance()->play(m_currentTrack);
    }
    else
    {
        QSpotifySession::instance()->setCurrentTrack(m_currentTrack);
    }
}

void QSpotifyTrackList::setShuffle(bool s)
{
    if (m_shuffle == s)
        return;

    m_shuffle = s;

    m_shuffleIndex = 0;

    if (m_shuffle)
    {
        generateShuffleList();
    }
}

void QSpotifyTrackList::generateShuffleList()
{
    int *values = new int[m_tracks.count()];
    int valuesSize = m_tracks.count();
    for (int i = 0; i < valuesSize; i++)
    {
        values[i] = i;
    }

    if (m_shuffleList.size() != m_tracks.count())
    {
        m_shuffleList.resize(m_tracks.count());
    }

    // Create the list of indexes
    qsrand(QTime::currentTime().msec());

    int shuffleListIndex = 0;

    bool currentTrackStillExists = m_currentTrack && m_tracks.contains(m_currentTrack);

    // If there is a current track we put it as the first
    if (currentTrackStillExists)
    {
        int currentTrackIndex = m_tracks.indexOf(m_currentTrack);
        m_shuffleList[0] = currentTrackIndex;
        values[currentTrackIndex] = values[valuesSize-1];
        --valuesSize;
        ++shuffleListIndex;
    }

    // Populate list
    while (valuesSize > 0)
    {
        int i = qrand() % valuesSize;
        m_shuffleList[shuffleListIndex] = values[i];
        values[i] = values[valuesSize-1];
        --valuesSize;
        ++shuffleListIndex;
    }

    delete values;
}

void QSpotifyTrackList::release()
{
    --m_refCount;
    if (m_refCount == 0)
    {
        delete(this);
    }
}

int QSpotifyTrackList::totalDuration() const
{
    qint64 total = 0;
    for (int i = 0; i < m_tracks.count(); ++i)
        total += m_tracks.at(i)->duration();

    return total;
}

int QSpotifyTrackList::nextAvailable(int i, bool ignoreShuffle)
{
    if (m_shuffle && !ignoreShuffle)
    {
        int shuffleIndex;

        // -1 means start at first track
        if (i != -1)
            shuffleIndex = m_shuffleList.indexOf(i);
        else
            shuffleIndex = i;

        do {
            ++shuffleIndex;
        } while (shuffleIndex < m_tracks.count() && !m_tracks.at(m_shuffleList[shuffleIndex])->isAvailable());

        if (shuffleIndex >= m_tracks.count())
        {
            // Reached the end
            i = shuffleIndex;
        }
        else
        {
            i = m_shuffleList[shuffleIndex];
        }
    }
    else
    {
        do {
            ++i;
        } while (i < m_tracks.count() && !m_tracks.at(i)->isAvailable());
    }
    return i;
}

int QSpotifyTrackList::previousAvailable(int i, bool ignoreShuffle)
{
    if (m_shuffle && !ignoreShuffle)
    {
        int shuffleIndex;

        // m_tracks.count means start at last track
        if (i != m_tracks.count())
            shuffleIndex = m_shuffleList.indexOf(i);
        else
            shuffleIndex = i;

        do {
            --shuffleIndex;
        } while (shuffleIndex >= 0 && !m_tracks.at(m_shuffleList[shuffleIndex])->isAvailable());

        if (shuffleIndex < 0)
        {
            // Reached the end
            i = shuffleIndex;
        }
        else
        {
            i = m_shuffleList[shuffleIndex];
        }
    }
    else
    {
        do {
            --i;
        } while (i > -1 && !m_tracks.at(i)->isAvailable());
    }

    return i;
}

void QSpotifyTrackList::addTracks(QVector<QSpotifyTrack*> & tracks, int pos)
{
    for (int i = 0; i < tracks.count(); i++)
    {
        if (pos == -1)
            m_tracks.append(tracks[i]);
        else
            m_tracks.insert(pos, tracks[i]);
        pos++;
    }

    if (m_currentTrack)
        m_currentIndex = m_tracks.indexOf(m_currentTrack);

    if (m_shuffle)
    {
        // Regenerate shuffle list
        generateShuffleList();
    }
}

QVector<QSpotifyTrack*> QSpotifyTrackList::removeTracks(QVector<int> & positions)
{
    QVector<QSpotifyTrack*> removedTracks;

    removedTracks.reserve(positions.count());

    for (int i = 0; i < positions.count(); i++)
    {
        int pos = positions.at(i);
        if (pos < 0 || pos >= m_tracks.count())
            continue;

        removedTracks.append(m_tracks[pos]);
        m_tracks[pos] = 0;
    }
    m_tracks.removeAll(0);

    // Update current index, making sure it doesn't stray into illegal values
    if (m_currentTrack)
    {
        int newIndex = m_tracks.indexOf(m_currentTrack);
        if (newIndex != -1)
            m_currentIndex = newIndex;
    }
    m_currentIndex = qMin(m_currentIndex, m_tracks.count() - 1);

    if (m_shuffle)
    {
        // Regenerate shuffle list
        generateShuffleList();
    }

    return removedTracks;
}
