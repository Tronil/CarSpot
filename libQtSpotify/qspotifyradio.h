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

#ifndef QSPOTIFYRADIO_HPP
#define QSPOTIFYRADIO_HPP

#include <QObject>
#include <QList>
#include <QTime>
#include "qspotifyartistbrowse.h"
#include "qspotifyartist.h"
#include "qspotifyalbumbrowse.h"
#include "qspotifytrack.h"

class QSpotifyRadio : public QObject
{
    Q_OBJECT
public:
    QSpotifyRadio(QSpotifyArtist & originalArtist);
    ~QSpotifyRadio();
    
    void nextSong();

    const QSpotifyArtist & getArtist() const;

    bool busy() const { return (m_playNextSong && !m_readyForNextRadioSong); }

Q_SIGNALS:
    void busyChanged();
    void errorOccured();

private Q_SLOTS:
    void origBrowserBusyChanged();
    void currentBrowserBusyChanged();
    void albumBrowserBusyChanged();

private:
    void selectSong();
    void findNextArtist();

    QSpotifyArtistBrowse m_origArtistBrowser;
    QSpotifyArtistBrowse m_currentArtistBrowser;
    QSpotifyAlbumBrowse m_albumBrowser;

    QList<QObject*> m_tracks;

    bool m_readyForNextRadioSong;
    bool m_playNextSong;
    int m_degreesOfSeperation;

    enum { MAX_DEGREES_OF_SEPERATION = 4 };
};

#endif // QSPOTIFYRADIO_HPP
