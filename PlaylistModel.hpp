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
#ifndef PLAYLISTMODEL_HPP
#define PLAYLISTMODEL_HPP

#include "libQtSpotify/qspotifyplaylist.h"
#include <QAbstractListModel>
#include "ListItem.hpp"

class PlaylistModel;

// Offline mode item of a playlist
class PlaylistOfflineItem : public ListItem
{
    Q_OBJECT

public:
    PlaylistOfflineItem(const QSpotifyPlaylist & playlist);

    virtual QVariant data(int role);

private slots:
    void offlineChanged();
    void playlistChanged();

private:
    QString getOfflineIcon();
    QString getOfflineText();
    QString getOfflineTextColor();

    const QSpotifyPlaylist & m_playlist;
};

class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QSpotifyPlaylist *playlist READ playlist WRITE setPlaylist NOTIFY playlistChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit PlaylistModel(QObject *parent = 0);
    ~PlaylistModel();
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    QSpotifyPlaylist *playlist() const { return m_playlist; }
    void setPlaylist(QSpotifyPlaylist *playlist);

    int count() const { return m_list.count(); }

Q_SIGNALS:
    void countChanged();
    void playlistChanged();
    
private slots:
    void itemChanged(int index);

private:
    void clear();

    void addHeader();
    void addOffline();
    void addTracks();

    QSpotifyPlaylist * m_playlist;
    QList<ListItem*> m_list;
};

#endif // PLAYLISTMODEL_HPP
