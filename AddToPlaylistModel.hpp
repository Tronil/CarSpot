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
#ifndef ADDTOPLAYLISTMODEL_HPP
#define ADDTOPLAYLISTMODEL_HPP

#include <QAbstractListModel>
#include "ListItem.hpp"
#include <qspotifyplaylist.h>
#include <qspotifyalbumbrowse.h>

class PlaylistItem : public ListItem
{
    Q_OBJECT

public:
    virtual QVariant data(int role);
    PlaylistItem(const QSpotifyPlaylist & playlist, int index);
    virtual ~PlaylistItem();

private slots:
    void playlistNameChanged();

private:
    const QSpotifyPlaylist & m_playlist;
};

class AddToPlaylistModel : public QAbstractListModel
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QList<QObject*> playlists READ playlists NOTIFY playlistsChanged)

public:
    explicit AddToPlaylistModel(QObject *parent = 0);
    ~AddToPlaylistModel();
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    int count() const { return m_list.count(); }

    QList<QObject*> playlists() const;

Q_SIGNALS:
    void countChanged();
    void playlistsChanged();

private slots:
    void itemDataChanged(int index);
    void playlistContainerDataChanged();

private:
    void populateList();
    void addAddAsNewItem();
    void addPlaylistItem(const QSpotifyPlaylist &playlist, int index);
    int countPlaylists();

    QList<QSpotifyPlaylist*> m_playlists;
    QList<ListItem*> m_list;
};

#endif // ADDTOPLAYLISTMODEL_HPP
