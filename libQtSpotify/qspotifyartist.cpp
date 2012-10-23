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

#include "qspotifyartist.h"
#include "qspotifyutils.h"
#include "qspotifyartistbrowse.h"

#include <QtCore/QDebug>
#include <libspotify/api.h>

QSpotifyArtist::QSpotifyArtist(sp_artist *artist)
    : QSpotifyObject(true)
{
    connect(this, SIGNAL(dataChanged()), this, SIGNAL(artistDataChanged()));

    sp_artist_add_ref(artist);
    m_sp_artist = artist;
    metadataUpdated();
}

QSpotifyArtist::~QSpotifyArtist()
{
    sp_artist_release(m_sp_artist);
}

bool QSpotifyArtist::isLoaded()
{
    return sp_artist_is_loaded(m_sp_artist);
}

bool QSpotifyArtist::updateData()
{
    bool updated = false;

    QString name = QString::fromUtf8(sp_artist_name(m_sp_artist));
    if (m_name != name) {
        m_name = name;
        updated = true;
    }

    if (m_pictureId.isEmpty()) {
        sp_link *link = sp_link_create_from_artist_portrait(m_sp_artist, SP_IMAGE_SIZE_NORMAL);
        if (link) {
            char buffer[200];
            int uriSize = sp_link_as_string(link, &buffer[0], 200);
            m_pictureId = QString::fromUtf8(&buffer[0], uriSize);
            sp_link_release(link);
            updated = true;
        }
    }

    return updated;
}

QString QSpotifyArtist::asLink() const
{
    QString string;
    sp_link *artist_link = sp_link_create_from_artist(m_sp_artist);

    string = QSpotifyUtils::splinkToString(*artist_link);

    sp_link_release(artist_link);

    return string;
}

QSpotifyArtist * QSpotifyArtist::fromLink(const QString & link)
{
    QSpotifyArtist *artist = 0;
    sp_link *artist_link = sp_link_create_from_string(link.toLocal8Bit().constData());
    sp_artist *spartist = sp_link_as_artist(artist_link);

    if (spartist)
    {
        artist = new QSpotifyArtist(spartist);
    }

    sp_link_release(artist_link);

    // When creating an artist from a link libspotify won't populate the data unless we
    // "perform a browse request", ie. create an artist browse object with that artist
    QSpotifyArtistBrowse hackBrowser;
    hackBrowser.setArtist(artist);
    // Note: We don't need to keep the object, the artist object will now get its data

    return artist;
}
