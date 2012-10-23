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
#include "WhatsNewModel.hpp"
#include "Constants.hpp"

AlbumItem::AlbumItem(QSpotifyAlbum & album, int index) :
    m_album(album),
    m_index(index)
{
    connect(&m_album, SIGNAL(albumDataChanged()), this, SLOT(albumDataChanged()));
}

void AlbumItem::albumDataChanged()
{
    emit dataChanged(m_index);
}

QVariant AlbumItem::data(int role)
{
    switch (role)
    {
    case TextRole:
        return m_album.name();

    case IdentifierRole:
        return m_index;

    case SecondRowTextRole:
        return m_album.artist();

    case SecondRowTextColorRole:
        return Constants::SpotifyGreen;

    case IconRole:
        return QLatin1String("image://svgElement/:/SpotifyAssets.svg/iconSpotifyListMusic");

    case SublistRole:
        return false;

    case DimmedRole:
        return false;

    default:
        return QVariant(QVariant::Invalid);
    }
}

WhatsNewModel::WhatsNewModel(QObject *parent) :
    QAbstractListModel(parent)
{
    setRoleNames(Constants::roleNames());

    // Search for new albums
    m_search.setQuery(QLatin1String("tag:new"));
    m_search.setTracksLimit(0);
    m_search.setArtistsLimit(0);
    m_search.setAlbumsLimit(15);

    connect(&m_search, SIGNAL(resultsChanged()), this, SLOT(resultsChanged()));
    m_search.search();
}

WhatsNewModel::~WhatsNewModel()
{
    qDeleteAll(m_list);
    m_list.clear();
}

void WhatsNewModel::resultsChanged()
{
    if (!m_list.empty())
    {
        beginRemoveRows(QModelIndex(), 0, m_list.count()-1);
        qDeleteAll(m_list);
        m_list.clear();
        endRemoveRows();
    }

    beginInsertRows(QModelIndex(), 0, m_search.albums().count()-1);
    for (int i = 0; i < m_search.albums().count(); i++)
    {
        AlbumItem *item = new AlbumItem(*(static_cast<QSpotifyAlbum*>(m_search.albums()[i])), i);
        connect(item, SIGNAL(dataChanged(int)), this, SLOT(itemDataChanged(int)));
        m_list.append(item);
    }
    endInsertRows();

    emit countChanged();
    emit albumsChanged();
}

int WhatsNewModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_list.count();
}

QVariant WhatsNewModel::data(const QModelIndex & index, int role) const
{
    if (index.row() < 0 || index.row() >= m_list.count())
        return QVariant(QVariant::Invalid);

    return m_list.at(index.row())->data(role);
}

void WhatsNewModel::itemDataChanged(int index)
{
    QModelIndex modelIndex = createIndex(index, 0);

    emit dataChanged(modelIndex, modelIndex);
}
