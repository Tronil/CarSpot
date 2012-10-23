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
import QtQuick 1.1

ListModel
{
    // What's New
    ListElement
    {
        icon: "image://svgElement/:/SpotifyAssets.svg/iconSpotifyWhatsNew"
        text: ""
        identifier: "WhatsNew"
        dimmed: false
        sublist: true
    }

    // Playlists
    ListElement
    {
        icon: "image://svgElement/:/SpotifyAssets.svg/iconSpotifyListPlaylists"
        text: ""
        identifier: "Playlists"
        dimmed: false
        sublist: true
    }

    // Radio
    ListElement
    {
        icon: "image://svgElement/:/SpotifyAssets.svg/iconSpotifyRadio"
        text: ""
        identifier: "Radio"
        dimmed: false
        sublist: true
    }

    // Settings
    ListElement
    {
        icon: "image://svgElement/:/SpotifyAssets.svg/iconSpotifySettings"
        text: ""
        identifier: "Settings"
        dimmed: false
        sublist: true
    }

    Component.onCompleted: translate()

    function translate()
    {
        var texts = [
                    //% "QTN_LIST_WHATS_NEW"
                    QT_TRID_NOOP("QTN_LIST_WHATS_NEW"),
                    //% "QTN_LIST_PLAYLISTS"
                    QT_TRID_NOOP("QTN_LIST_PLAYLISTS"),
                    //% "QTN_LIST_RADIO"
                    QT_TRID_NOOP("QTN_LIST_RADIO"),
                    //% "QTN_LIST_SETTINGS"
                    QT_TRID_NOOP("QTN_LIST_SETTINGS")
                ]
        var i = 0
        while (i < count)
        {
            setProperty(i, "text", qsTrId(texts[i]))
            i++
        }
    }
}
