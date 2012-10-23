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
#ifndef KEYEVENTS_HPP
#define KEYEVENTS_HPP

#include <QObject>
#include <qmsystem2/qmkeys.h>

class KeyEvents : public QObject
{
    Q_OBJECT
    Q_ENUMS(Keys)

public:
    enum Keys {
        PlayPause,
        Stop,
        Forward,
        Rewind,
        NextSong,
        PreviousSong,
        Pause,
        Play
    };

    explicit KeyEvents();

Q_SIGNALS:
    void keyPressed(int key);
    
private slots:
    void keyEvent(MeeGo::QmKeys::Key qmKey, MeeGo::QmKeys::State state);
    void terminalModeKeyEvent(bool down, uint tmKey);

private:
    // Terminal mode (media) key values (see terminal mode specification)
    enum terminalModeKeys {
        Multimedia_Play = 0x30000400,
        Multimedia_Pause = 0x30000401,
        Multimedia_Stop = 0x30000402,
        Multimedia_Forward = 0x30000403,
        Multimedia_Rewind = 0x30000404,
        Multimedia_Next = 0x30000405,
        Multimedia_Previous = 0x30000406,
        Multimedia_Mute = 0x30000407,
        Multimedia_Unmute = 0x30000408
    };

    MeeGo::QmKeys m_qmKeys;
};

#endif // KEYEVENTS_HPP
