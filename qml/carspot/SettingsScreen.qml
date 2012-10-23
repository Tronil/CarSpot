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
    id: settings_list

    listModel: SettingsModel { id: settings_model}

    onListItemClicked:
    {
        switch (item)
        {
        case "Offline":
            offline()
            break

        case "Stream quality":
            streamQuality()
            break

        case "Offline sync":
            offlineSync()
            break

        case "Sync over mobile":
            syncOverMobile()
            break

        case "Logout":
            logout()
            break

        case "About":
            about()
            break
        }
    }

    function offline()
    {
        spotifySession.setOfflineMode(!spotifySession.offlineMode, false)
    }

    function streamQuality()
    {
        startSubList("StreamQualityScreen.qml")
    }

    function offlineSync()
    {
        startSubList("OfflineSyncScreen.qml")
    }

    function syncOverMobile()
    {
        spotifySession.syncOverMobile = !spotifySession.syncOverMobile
    }

    function logout()
    {
        spotifySession.logout(false);
        optionsDone()
    }

    function about()
    {
            root_window.pageStack.push(Qt.resolvedUrl("AboutScreen.qml"), {backStepTo: settings_list}, true)
    }

    function updateStreamingQuality()
    {
        if (spotifySession.streamingQuality == SpotifySession.LowQuality)
        {
            //% "QTN_LIST_LOW_BW"
            settings_model.setProperty(1, "second_row_text", qsTrId("QTN_LIST_LOW_BW"))
        }
        else
        {
            //% "QTN_LIST_HIGH_BW"
            settings_model.setProperty(1, "second_row_text", qsTrId("QTN_LIST_HIGH_BW"))
        }
    }

    function updateOfflineQuality()
    {
        if (spotifySession.syncQuality == SpotifySession.LowQuality)
        {
            //% "QTN_LIST_LOW_QUALITY"
            settings_model.setProperty(2, "second_row_text", qsTrId("QTN_LIST_LOW_QUALITY"))
        }
        else
        {
            //% "QTN_LIST_HIGH_QUALITY"
            settings_model.setProperty(2, "second_row_text", qsTrId("QTN_LIST_HIGH_QUALITY"))
        }
    }

    function startSubList(component_file)
    {
        var new_page = root_window.pageStack.push(Qt.resolvedUrl(component_file), {backStepTo: settings_list}, true)
        new_page.optionsDone.connect(optionsDone)
    }

    Connections
    {
        target: spotifySession
        onOfflineModeChanged: settings_model.setProperty(0, "toggle_state", spotifySession.offlineMode)
        onStreamingQualityChanged: updateStreamingQuality()
        onSyncQualityChanged: updateOfflineQuality()
        onSyncOverMobileChanged: settings_model.setProperty(3, "toggle_state", spotifySession.syncOverMobile)
    }

    Component.onCompleted:
    {
        // Offline mode
        settings_model.setProperty(0, "toggle_state", spotifySession.offlineMode)

        // Streaming quality
        updateStreamingQuality()

        // Offline sync quality
        updateOfflineQuality()

        // Sync over mobile
        settings_model.setProperty(3, "toggle_state", spotifySession.syncOverMobile)
    }
}
