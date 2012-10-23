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
import QtSpotify 1.0

CarModeList
{
    id: options_list

    listModel: OptionsModel {}

    onListItemClicked:
    {
        switch (item)
        {
        case "WhatsNew":
            if (!listModel.get(0).dimmed)
                startSubList("WhatsNewScreen.qml")
            break

        case "Playlists":
            if (!listModel.get(1).dimmed)
                startSubList("PlaylistsScreen.qml")
            break

        case "Radio":
            if (!listModel.get(2).dimmed)
                startSubList("RadioScreen.qml")
            break

        case "Settings":
            startSubList("SettingsScreen.qml")
            break
        }
    }

    function startSubList(component_file)
    {
        var new_page = root_window.pageStack.push(Qt.resolvedUrl(component_file), {backStepTo: options_list}, true)
        new_page.optionsDone.connect(optionsDone)
    }

    Component.onCompleted:
    {
        // Check for dimming
        checkWhatsNewDimming()

        if (spotifySession.user.playlists.length == 2 && spotifySession.user.playlists[0].tracks.length == 0
                && spotifySession.user.playlists[1].tracks.length)
        {
            // No tracks to play, dimm playlists
            listModel.setProperty(1, "dimmed", true)
        }

        checkRadioDimming()
    }

    function checkWhatsNewDimming()
    {
        if (spotifySession.offlineMode)
        {
            // Dimm What's New in offline mode
            listModel.setProperty(0, "dimmed", true)
        }
        else
        {
            listModel.setProperty(0, "dimmed", false)
        }
    }

    function checkRadioDimming()
    {
        if (spotifySession.lastArtists.length == 0 || spotifySession.offlineMode)
        {
            // No radios to play, dimm
            listModel.setProperty(2, "dimmed", true)
        }
        else
        {
            listModel.setProperty(2, "dimmed", false)
        }
    }

    Connections
    {
        target: spotifySession

        onOfflineModeChanged:
        {
            checkWhatsNewDimming()
            checkRadioDimming()
        }

    }
}


