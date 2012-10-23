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
#include "NewAlbumModel.hpp"
#include "Constants.hpp"

// Header item of a new album
class AlbumHeaderItem : public ListItem
{
public:
    AlbumHeaderItem(const QString & title, const QString & artist);

    virtual QVariant data(int role);

private:
    QString m_header;
};

AlbumHeaderItem::AlbumHeaderItem(const QString & artist, const QString & title) :
    ListItem(0),
    m_header(artist + QLatin1String(" | ") + title)
{
}

QVariant AlbumHeaderItem::data(int role)
{
    switch (role)
    {
    case TextRole:
        return m_header;

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

// "Add to playlist" item of a new album
class AddToPlaylistItem : public ListItem
{
public:
    AddToPlaylistItem();

    virtual QVariant data(int role);
};

AddToPlaylistItem::AddToPlaylistItem() :
    ListItem(1)
{}

QVariant AddToPlaylistItem::data(int role)
{
    switch (role)
    {
    case TextRole:
        //% "QTN_LIST_ADD_TO_PLAYLIST"
        return qtTrId("QTN_LIST_ADD_TO_PLAYLIST");

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


NewAlbumModel::NewAlbumModel(QObject *parent) :
    QAbstractListModel(parent),
    m_albumBrowser(0)
{
    setRoleNames(Constants::roleNames());
}

NewAlbumModel::~NewAlbumModel()
{
    qDeleteAll(m_list);
    m_list.clear();
}

void NewAlbumModel::setAlbumBrowser(QSpotifyAlbumBrowse *albumBrowser)
{
    if (albumBrowser == m_albumBrowser)
        return;

    m_albumBrowser = albumBrowser;

    if (!m_list.empty())
    {
        beginRemoveRows(QModelIndex(), 0, m_list.count()-1);
        qDeleteAll(m_list);
        m_list.clear();
        endRemoveRows();
    }

    if (m_albumBrowser->busy())
    {
        albumBrowserBusyChanged();
    }
    else
    {
        connect(m_albumBrowser, SIGNAL(busyChanged()), this, SLOT(albumBrowserBusyChanged()));
    }
}

void NewAlbumModel::albumBrowserBusyChanged()
{
    if (!m_albumBrowser->busy())
    {
        disconnect(m_albumBrowser, SIGNAL(busyChanged()), this, SLOT(albumBrowserBusyChanged()));

        // Populate list
        beginInsertRows(QModelIndex(), 0, m_albumBrowser->tracks().count() + 2 - 1);
        addHeader();
        addAddToPlaylist();
        addTracks();
        endInsertRows();

        emit albumBrowserChanged();
        emit countChanged();
    }
}

void NewAlbumModel::addHeader()
{
    m_list.append(new AlbumHeaderItem(m_albumBrowser->album()->artist(), m_albumBrowser->album()->name()));
}

void NewAlbumModel::addAddToPlaylist()
{
    m_list.append(new AddToPlaylistItem());
}

void NewAlbumModel::addTracks()
{
    for (int i = 0; i < m_albumBrowser->tracks().count(); i++)
    {
        ListTrackItem *item = new ListTrackItem(i+2, *(static_cast<QSpotifyTrack*>(m_albumBrowser->tracks()[i])));
        connect(item, SIGNAL(dataChanged(int)), this, SLOT(itemChanged(int)));
        m_list.append(item);
    }
}

void NewAlbumModel::itemChanged(int index)
{
    QModelIndex modelIndex = createIndex(index, 0);

    emit dataChanged(modelIndex, modelIndex);
}

int NewAlbumModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_list.count();
}

QVariant NewAlbumModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_list.count())
        return QVariant(QVariant::Invalid);

    return m_list.at(index.row())->data(role);
}
