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
#include "libQtSpotify/qspotifysession.h"
#include "libQtSpotify/qspotifyplaylistcontainer.h"
#include "PlaylistsModel.hpp"
#include "Constants.hpp"

// List item class
PlaylistsItem::PlaylistsItem(int index, const QSpotifyPlaylist & playlist) :
    QObject(0),
    m_index(index),
    m_playlist(playlist),
    m_trackCount(playlist.trackCount()),
    m_dimmed(false)
{
    m_dimmed = getDimmed();

    // Set the playlist name
    switch (m_playlist.type())
    {
    case QSpotifyPlaylist::Inbox:
        //% "QTN_LIST_INBOX"
        m_name = qtTrId("QTN_LIST_INBOX");
        break;

    case QSpotifyPlaylist::Starred:
        //% "QTN_LIST_STARRED"
        m_name = qtTrId("QTN_LIST_STARRED");
        break;

    default:
        m_name = playlist.name();
        break;
    }

    connect(&m_playlist, SIGNAL(dataChanged()), this, SLOT(playlistDataChanged()));
    connect(QSpotifySession::instance(), SIGNAL(offlineModeChanged()), this, SLOT(offlineModeChanged()));
}

QVariant PlaylistsItem::data(int role)
{
    switch (role)
    {
    case TextRole:
        return m_name;

    case IconRole:
        return getIcon();

    case SecondRowTextRole:
        //% "QTN_LIST_TRACK_NUM"
        return qtTrId("QTN_LIST_TRACK_NUM").arg(m_trackCount);

    case IdentifierRole:
        return m_index;

    case SublistRole:
        return (m_trackCount > 0);

    case DimmedRole:
        return m_dimmed;

    default:
        return QVariant(QVariant::Invalid);
    }
}

bool PlaylistsItem::getDimmed()
{
    bool dimmed;

    if (m_trackCount > 0)
    {
        if (QSpotifySession::instance()->offlineMode() && m_playlist.hasOfflineTracks() == false)
        {
            dimmed = true;
        }
        else
        {
            dimmed = false;
        }
    }
    else
    {
        dimmed = true;
    }

    return dimmed;
}

QString PlaylistsItem::getIcon()
{
    switch (m_playlist.type())
    {
    case QSpotifyPlaylist::Inbox:
        return QLatin1String("image://svgElement/:/SpotifyAssets.svg/iconSpotifyInbox");

    case QSpotifyPlaylist::Starred:
        return QLatin1String("image://svgElement/:/SpotifyAssets.svg/iconSpotifyStarredTracks");

    default:
        // Normal playlist
        return QLatin1String("image://svgElement/:/SpotifyAssets.svg/iconSpotifyListMusic");
    }
}

void PlaylistsItem::playlistDataChanged()
{
    bool changed = false;

    if (m_playlist.type() == QSpotifyPlaylist::Playlist && m_name != m_playlist.name())
    {
        m_name = m_playlist.name();
        changed = true;
    }

    if (m_trackCount != m_playlist.trackCount())
    {
        m_trackCount = m_playlist.trackCount();
        changed = true;
    }

    if (m_dimmed != getDimmed())
    {
        m_dimmed = getDimmed();
        changed = true;
    }

    if (changed)
    {
        emit dataChanged(m_index);
    }
}

void PlaylistsItem::offlineModeChanged()
{
    if (m_dimmed != getDimmed())
    {
        m_dimmed = getDimmed();
        emit dataChanged(m_index);
    }
}

PlaylistsModel::PlaylistsModel(QObject *parent) :
    QAbstractListModel(parent),
    m_user(0)
{
    setRoleNames(Constants::roleNames());
}

PlaylistsModel::~PlaylistsModel()
{
    clear();
}

void PlaylistsModel::clear()
{
    qDeleteAll(m_list);
    m_list.clear();
}

void PlaylistsModel::setUser(QSpotifyUser *user)
{
    if (m_user == user)
        return;

    clear();

    m_user = user;

    beginInsertRows(QModelIndex(), 0 , user->playlists().count() + 2 - 1);

    // Inbox
    m_list.append(new PlaylistsItem(0, *user->inbox()));
    connect(m_list[0], SIGNAL(dataChanged(int)), this, SLOT(itemChanged(int)));

    // Starred tracks
    m_list.append(new PlaylistsItem(1, *user->starredList()));
    connect(m_list[1], SIGNAL(dataChanged(int)), this, SLOT(itemChanged(int)));

    // User generated playlists
    addUserPlaylists();

    endInsertRows();

    connect(m_user->playlistContainer(), SIGNAL(playlistsAdded()), this, SLOT(playlistsChanged()));
    connect(m_user->playlistContainer(), SIGNAL(playlistsRemoved()), this, SLOT(playlistsChanged()));
    connect(QSpotifySession::instance(), SIGNAL(offlineModeChanged()), this, SLOT(playlistsChanged()));

    emit userChanged();
    emit countChanged();
}

void PlaylistsModel::addUserPlaylists()
{
    QList<QObject*> playlists = m_user->playlistsAsQObject();

    for (int i = 2; i < playlists.count(); i++)
    {
        PlaylistsItem *item = new PlaylistsItem(i, *(static_cast<QSpotifyPlaylist*>(playlists[i])));
        m_list.append(item);
        connect(item, SIGNAL(dataChanged(int)), this, SLOT(itemChanged(int)));
    }
}

void PlaylistsModel::itemChanged(int index)
{
    QModelIndex modelIndex = createIndex(index, 0);

    emit dataChanged(modelIndex, modelIndex);
}

void PlaylistsModel::playlistsChanged()
{
    bool changed = false;

    // Check if anything has changed
    if (m_user->playlists().count() + 2 != m_list.count())
    {
        changed = true;
    }
    else
    {
        QList<QObject*> playlists = m_user->playlistsAsQObject();
        for (int i = 2; i < playlists.count(); i++)
        {
            if (&(m_list[i]->m_playlist) != playlists[i])
            {
                changed = true;
                break;
            }
        }
    }

    if (changed)
    {
        beginRemoveRows(QModelIndex(), 2, m_list.count() - 1);

        // Regenerate list (the 2 first items are always Inbox and Starred)
        while (m_list.count() > 2)
        {
            delete(m_list.takeLast());
        }

        endRemoveRows();

        beginInsertRows(QModelIndex(), 2, m_user->playlists().count() + 2 - 1);

        addUserPlaylists();

        endInsertRows();

        emit countChanged();
    }
}

int PlaylistsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_list.count();
}

QVariant PlaylistsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_list.count())
        return QVariant(QVariant::Invalid);

    return m_list.at(index.row())->data(role);
}
