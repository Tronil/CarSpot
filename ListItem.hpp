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
#ifndef LISTITEM_HPP
#define LISTITEM_HPP

#include <QObject>
#include <qspotifytrack.h>

// Base class for list items containing only an index
class ListItem : public QObject
{
    Q_OBJECT

public:
    virtual QVariant data(int) = 0;
    virtual ~ListItem() {}

Q_SIGNALS:
    void dataChanged(int index);

protected:
    ListItem(int index);

    const int m_index;
};

// Track item of a list
class ListTrackItem : public ListItem
{
    Q_OBJECT

public:
    ListTrackItem(int index, QSpotifyTrack & track);

    virtual QVariant data(int role);

    virtual ~ListTrackItem();

Q_SIGNALS:
    void dataChanged(int index);

private slots:
    void trackOfflineStatusChanged();
    void offlineModeChanged();

private:
    bool getDimmed();

    QSpotifyTrack & m_track;
    bool m_offline;
    bool m_dimmed;
};


#endif // LISTITEM_HPP
