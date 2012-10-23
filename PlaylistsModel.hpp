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
#ifndef PLAYLISTSMODEL_HPP
#define PLAYLISTSMODEL_HPP

#include <QAbstractListModel>
#include "libQtSpotify/qspotifyuser.h"
#include "libQtSpotify/qspotifyplaylist.h"

class PlaylistsItem : public QObject
{
    Q_OBJECT

public:
    explicit PlaylistsItem(int index, const QSpotifyPlaylist & playlist);

    QVariant data(int role);

Q_SIGNALS:
    void dataChanged(int index);

private slots:
    void offlineModeChanged();
    void playlistDataChanged();

private:
    bool getDimmed();
    QString getIcon();

    int m_index;
    const QSpotifyPlaylist & m_playlist;
    QString m_name;
    int m_trackCount;
    bool m_dimmed;

    friend class PlaylistsModel;
};

class PlaylistsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QSpotifyUser *user READ user WRITE setUser NOTIFY userChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit PlaylistsModel(QObject *parent = 0);
    ~PlaylistsModel();
    
    QSpotifyUser *user() const { return m_user; }
    void setUser(QSpotifyUser *user);
    int count() const { return m_list.count(); }

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

signals:
    void userChanged();
    void countChanged();

public slots:
    void itemChanged(int index);
    void playlistsChanged();

private:
    void clear();
    void addUserPlaylists();

    QSpotifyUser *m_user;
    QList<PlaylistsItem*> m_list;
};

#endif // PLAYLISTSMODEL_HPP
