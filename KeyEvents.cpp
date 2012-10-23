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
#include "KeyEvents.hpp"
#include <QtDBus/QDBusConnection>

KeyEvents::KeyEvents() :
    QObject(0),
    m_qmKeys(0)
{
    connect(&m_qmKeys, SIGNAL(keyEvent(MeeGo::QmKeys::Key,MeeGo::QmKeys::State)), this, SLOT(keyEvent(MeeGo::QmKeys::Key,MeeGo::QmKeys::State)));
    QDBusConnection::sessionBus().connect(QLatin1String("com.nokia.TerminalModeKeys"), QLatin1String("/"),
                                          QLatin1String("com.nokia.TerminalModeKeys"), QLatin1String("tmKeyEvent"),
                                          this, SLOT(terminalModeKeyEvent(bool,uint)));
}

void KeyEvents::keyEvent(MeeGo::QmKeys::Key qmKey, MeeGo::QmKeys::State state)
{
    if (state == MeeGo::QmKeys::KeyDown)
    {
        Keys key;

        switch(qmKey)
        {
        case MeeGo::QmKeys::PlayPause:
            key = PlayPause;
            break;

        case MeeGo::QmKeys::Stop:
            key = Stop;
            break;

        case MeeGo::QmKeys::Forward:
            key = Forward;
            break;

        case MeeGo::QmKeys::Rewind:
            key = Rewind;
            break;

        case MeeGo::QmKeys::NextSong:
            key = NextSong;
            break;

        case MeeGo::QmKeys::PreviousSong:
            key = PreviousSong;
            break;

        case MeeGo::QmKeys::Pause:
            key = Pause;
            break;

        case MeeGo::QmKeys::Play:
            key = Play;
            break;

        default:
            // Unhandled key
            return;
        }

        emit keyPressed(key);
    }
}

void KeyEvents::terminalModeKeyEvent(bool down, uint tmKey)
{
    if (down == true)
    {
        Keys key;

        switch (tmKey)
        {
        case Multimedia_Play:
            key = Play;
            break;

        case Multimedia_Pause:
            key = Pause;
            break;

        case Multimedia_Stop:
            key = Stop;
            break;

        case Multimedia_Forward:
            key = Forward;
            break;

        case Multimedia_Rewind:
            key = Rewind;
            break;

        case Multimedia_Next:
            key = NextSong;
            break;

        case Multimedia_Previous:
            key = PreviousSong;
            break;

        case Multimedia_Mute:
        case Multimedia_Unmute:
        default:
            // Unhandled key
            return;
        }

        emit keyPressed(key);
    }
}
