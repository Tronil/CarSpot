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
#include "qspotifysession.h"
#include "qspotifyuser.h"
#include "qspotifyplaylistcontainer.h"
#include "AddToPlaylistModel.hpp"
#include "Constants.hpp"

class AddAsNewPlaylistItem : public ListItem
{
public:
    virtual QVariant data(int role);
    AddAsNewPlaylistItem();
    virtual ~AddAsNewPlaylistItem();
};

AddAsNewPlaylistItem::AddAsNewPlaylistItem() :
    ListItem(0)
{}

AddAsNewPlaylistItem::~AddAsNewPlaylistItem()
{}

QVariant AddAsNewPlaylistItem::data(int role)
{
    switch (role)
    {
    case TextRole:
        //% "QTN_LIST_ADD_AS_NEW_PLAYLIST"
        return qtTrId("QTN_LIST_ADD_AS_NEW_PLAYLIST");

    case IdentifierRole:
        return m_index;

    case IconRole:
        return QLatin1String("image://svgElement/:/SpotifyAssets.svg/iconSpotifyPlus");

    case SublistRole:
        return false;

    default:
        return QVariant(QVariant::Invalid);
    }
}

PlaylistItem::PlaylistItem(const QSpotifyPlaylist &playlist, int index) :
    ListItem(index),
    m_playlist(playlist)
{
    connect(&playlist, SIGNAL(nameChanged()), this, SLOT(playlistNameChanged()));
}

PlaylistItem::~PlaylistItem()
{}

QVariant PlaylistItem::data(int role)
{
    switch (role)
    {
    case TextRole:
        return m_playlist.name();

    case IdentifierRole:
        return m_index;

    case IconRole:
        return QLatin1String("image://svgElement/:/SpotifyAssets.svg/iconSpotifyListMusic");

    case SublistRole:
        return false;

    default:
        return QVariant(QVariant::Invalid);
    }
}

void PlaylistItem::playlistNameChanged()
{
    emit dataChanged(m_index);
}

AddToPlaylistModel::AddToPlaylistModel(QObject *parent) :
    QAbstractListModel(parent),
    m_playlists(QSpotifySession::instance()->user()->playlists())
{
    setRoleNames(Constants::roleNames());

    connect(QSpotifySession::instance()->user()->playlistContainer(), SIGNAL(playlistContainerDataChanged()),
            this, SLOT(playlistContainerDataChanged()));

    populateList();
}

AddToPlaylistModel::~AddToPlaylistModel()
{
    qDeleteAll(m_list);
    m_list.clear();
}

void AddToPlaylistModel::playlistContainerDataChanged()
{
    // Re-populate model
    beginRemoveRows(QModelIndex(), 0, m_list.count() - 1);
    qDeleteAll(m_list);
    m_list.clear();
    endRemoveRows();

    m_playlists = QSpotifySession::instance()->user()->playlists();

    populateList();
}

void AddToPlaylistModel::itemDataChanged(int index)
{
    QModelIndex modelIndex = createIndex(index, 0);

    emit dataChanged(modelIndex, modelIndex);
}

void AddToPlaylistModel::populateList()
{
    beginInsertRows(QModelIndex(), 0, 1 + countPlaylists() - 1);
    addAddAsNewItem();
    for (int i = 0; i < m_playlists.count(); i++)
    {
        if (m_playlists[i]->modifiable())
            addPlaylistItem(*(m_playlists[i]), i+1);
    }
    endInsertRows();

    emit playlistsChanged();
    emit countChanged();
}

int AddToPlaylistModel::countPlaylists()
{
    int count = 0;

    for (int i = 0; i < m_playlists.count(); i++)
    {
        if (m_playlists[i]->modifiable())
            count++;
    }

    return count;
}

void AddToPlaylistModel::addAddAsNewItem()
{
    AddAsNewPlaylistItem *item = new AddAsNewPlaylistItem;
    connect(item, SIGNAL(dataChanged(int)), this, SLOT(itemDataChanged(int)));
    m_list.append(item);
}

void AddToPlaylistModel::addPlaylistItem(const QSpotifyPlaylist &playlist, int index)
{
    PlaylistItem *item = new PlaylistItem(playlist, index);
    connect(item, SIGNAL(dataChanged(int)), this, SLOT(itemDataChanged(int)));
    m_list.append(item);
}

int AddToPlaylistModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_list.count();
}

QVariant AddToPlaylistModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_list.count())
        return QVariant(QVariant::Invalid);

    return m_list.at(index.row())->data(role);
}

QList<QObject*> AddToPlaylistModel::playlists() const
{
    QList<QObject*> list;

    for (int i = 0; i < m_playlists.count(); i++)
        list.append(static_cast<QObject*>(m_playlists.at(i)));

    return list;
}
