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
#include <QObject>
#include "PlaylistModel.hpp"
#include "libQtSpotify/qspotifytrack.h"
#include "libQtSpotify/qspotifysession.h"
#include "Constants.hpp"

// Header item of a playlist
class PlaylistHeaderItem : public ListItem
{
public:
    PlaylistHeaderItem(const QString & title);

    virtual QVariant data(int role);

    virtual ~PlaylistHeaderItem();

private:
    QString m_title;
};

PlaylistHeaderItem::PlaylistHeaderItem(const QString & title) :
    ListItem(0),
    m_title(title)
{
}

PlaylistHeaderItem::~PlaylistHeaderItem()
{
}

QVariant PlaylistHeaderItem::data(int role)
{
    switch (role)
    {
    case TextRole:
        return m_title;

    case IdentifierRole:
        return m_index;

    case IconRole:
        return QLatin1String("image://svgElement/:/SpotifyAssets.svg/iconSpotifyListPlay");

    case SublistRole:
        return false;

    case BackgroundRole:
        return QLatin1String("image://svgElement/:/SpotifyAssets.svg/listBgSpotify-header_2_");

    default:
        return QVariant(QVariant::Invalid);
    }
}

// Offline mode item of a playlist
PlaylistOfflineItem::PlaylistOfflineItem(const QSpotifyPlaylist & playlist)
    : ListItem(1),
      m_playlist(playlist)
{
    connect(&playlist, SIGNAL(offlineStatusChanged()), this, SLOT(offlineChanged()));
    connect(&playlist, SIGNAL(playlistDataChanged()), this, SLOT(playlistChanged()));
}

QVariant PlaylistOfflineItem::data(int role)
{
    switch (role)
    {
    case TextRole:
        //% "QTN_LIST_OFFLINE_PLAYLIST"
        return qtTrId("QTN_LIST_OFFLINE_PLAYLIST");

    case IdentifierRole:
        return m_index;

    case IconRole:
        return getOfflineIcon();

    case SublistRole:
        return false;

    case SecondRowTextRole:
        return getOfflineText();

    case SecondRowTextColorRole:
        return getOfflineTextColor();

    case ToggleItemRole:
        return true;

    case ToggleStateRole:
        return m_playlist.availableOffline();

    default:
        return QVariant(QVariant::Invalid);
    }
}

QString PlaylistOfflineItem::getOfflineIcon()
{
    switch (m_playlist.offlineStatus())
    {
    case QSpotifyPlaylist::No:
        return QLatin1String("image://svgElement/:/SpotifyAssets.svg/iconSpotifyDownloadInactive");

    case QSpotifyPlaylist::Yes:
        return QLatin1String("image://svgElement/:/SpotifyAssets.svg/iconSpotifyDownloadDone");

    case QSpotifyPlaylist::Downloading:
    case QSpotifyPlaylist::Waiting:
        return QLatin1String("image://svgElement/:/SpotifyAssets.svg/iconSpotifyDownloadActive");
    }
}

QString PlaylistOfflineItem::getOfflineText()
{
    switch (m_playlist.offlineStatus())
    {
    case QSpotifyPlaylist::No:
        //% "QTN_LIST_OFF"
        return qtTrId("QTN_LIST_OFF");

    case QSpotifyPlaylist::Yes:
        //% "QTN_LIST_ON"
        return qtTrId("QTN_LIST_ON");

    case QSpotifyPlaylist::Downloading:
        //% "QTN_LIST_DOWNLOADING"
        return qtTrId("QTN_LIST_DOWNLOADING").arg(m_playlist.offlineDownloadProgress());

    case QSpotifyPlaylist::Waiting:
        //% "QTN_LIST_WAITING"
        return qtTrId("QTN_LIST_WAITING");
    }
}

QString PlaylistOfflineItem::getOfflineTextColor()
{
    if (m_playlist.offlineStatus() == QSpotifyPlaylist::No || m_playlist.offlineStatus() == QSpotifyPlaylist::Yes)
    {
        return Constants::SpotifyGreen;
    }
    else
    {
        return Constants::DimmedTextColor;
    }
}

void PlaylistOfflineItem::offlineChanged()
{
    emit dataChanged(m_index);
}

void PlaylistOfflineItem::playlistChanged()
{
    if (m_playlist.offlineStatus() == QSpotifyPlaylist::Downloading)
        emit dataChanged(m_index);
}



PlaylistModel::PlaylistModel(QObject *parent) :
    QAbstractListModel(parent)
{
    setRoleNames(Constants::roleNames());
}

PlaylistModel::~PlaylistModel()
{
    clear();
}

void PlaylistModel::setPlaylist(QSpotifyPlaylist *playlist)
{
    if (playlist == m_playlist)
        return;

    clear();

    m_playlist = playlist;

    // Construct list
    beginInsertRows(QModelIndex(), 0, m_playlist->trackCount() - 1 + 2);
    addHeader();
    addOffline();
    addTracks();
    endInsertRows();

    emit playlistChanged();
    emit countChanged();
}

void PlaylistModel::clear()
{
    qDeleteAll(m_list);
    m_list.clear();
}

void PlaylistModel::addHeader()
{
    QString title;
    PlaylistHeaderItem *header;

    switch (m_playlist->type())
    {
    case QSpotifyPlaylist::Inbox:
        //% "QTN_LIST_INBOX"
        title = qtTrId("QTN_LIST_INBOX");
        break;

    case QSpotifyPlaylist::Starred:
        //% "QTN_LIST_STARRED"
        title = qtTrId("QTN_LIST_STARRED");
        break;

    case QSpotifyPlaylist::Playlist:
        title = m_playlist->name();
        break;
    }

    header = new PlaylistHeaderItem(title);

    m_list.append(header);
}

void PlaylistModel::addOffline()
{
    PlaylistOfflineItem *offline = new PlaylistOfflineItem(*m_playlist);

    connect(offline, SIGNAL(dataChanged(int)), this, SLOT(itemChanged(int)));

    m_list.append(offline);
}

void PlaylistModel::addTracks()
{
    QList<QObject*> tracks = m_playlist->tracksAsQObject();

    int i = 0;
    while (i < tracks.count())
    {
        ListTrackItem *track = new ListTrackItem(i+2, *(static_cast<QSpotifyTrack*>(tracks[i])));
        m_list.append(track);
        connect(track, SIGNAL(dataChanged(int)), this, SLOT(itemChanged(int)));
        ++i;
    }
}

int PlaylistModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return m_list.count();
}


QVariant PlaylistModel::data(const QModelIndex & index, int role) const
{
    if (index.row() < 0 || index.row() >= m_list.count())
        return QVariant(QVariant::Invalid);

    return m_list.at(index.row())->data(role);
}

void PlaylistModel::itemChanged(int index)
{
    QModelIndex modelIndex = createIndex(index, 0);

    emit dataChanged(modelIndex, modelIndex);
}
