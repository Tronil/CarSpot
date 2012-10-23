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
import com.nokia.meego 1.1     // MeeGo 1.2 Harmattan components
import carspot 1.0

CarModePage
{
    id: playing_screen

    signal openOptions

    disableBackButton: true

    // HW keys handling
    HWKeys {}

    // Sound resource handling
    Connections
    {
        target: platformWindow

        // Keep the audio resource when the CarSpot window is active
        // (so volume control will control the music playback volume)
        onActiveChanged: spotifySession.keepResources = platformWindow.active
    }

    WaitAnimation
    {
        z: 1
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: controls_bg.top

        visible: (spotifySession.radioMode && spotifySession.radioBusy) ? true : false
    }

    // Play controls
    Image
    {
        id: controls_bg
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        height: Constants.ControlsHeight

        source: "image://svgElement/:/SpotifyAssets.svg/playCtrlsBg"

        // Previous song
        SVGImageButton
        {
            id: previous_button
            anchors.left: parent.left
            height: parent.height
            width: Constants.ControlWidth
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            icon: "SpotifyAssets.svg/iconBtnPrev"

            onClicked: spotifySession.playPrevious()

            enabled: (spotifySession.radioMode) ? false : true

            // Dimm when in radio mode
            Rectangle
            {
                anchors.fill: parent
                color: "black"
                opacity: 0.7
                visible: spotifySession.radioMode
            }
        }

        // Play/Pause
        SVGImageButton
        {
            id: play_button
            anchors.left: previous_button.right
            height: parent.height
            width: Constants.ControlWidth
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            states : [
                State
                {
                    name: "Paused"
                    PropertyChanges
                    {
                        target: play_button
                        icon: "SpotifyAssets.svg/iconBtnPlay"
                    }
                },
                State
                {
                    name: "Playing"
                    PropertyChanges
                    {
                        target: play_button
                        icon: "SpotifyAssets.svg/iconBtnPause"
                    }
                }
            ]

            state: spotifySession.isPlaying ? "Playing" : "Paused"

            onClicked: if (state == "Paused")
                       {
                           spotifySession.resume()
                       }
                       else
                       {
                           spotifySession.pause()
                       }
        }

        // Next song
        SVGImageButton
        {
            id: next_button
            anchors.left: play_button.right
            height: parent.height
            width: Constants.ControlWidth
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            icon: "SpotifyAssets.svg/iconBtnNext"

            onClicked: spotifySession.playNext()
        }

        // Options
        SVGImageButton
        {
            id: options_btn
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: next_button.right

            background: "SpotifyAssets.svg/btnBgLibrary-normal"

            icon: "SpotifyAssets.svg/iconLibrary"

            onClicked: playing_screen.openOptions()
        }

    }


    // Play time indicator
    Image
    {
        id: play_indicator_bg
        anchors.left: controls_bg.left
        anchors.right: parent.right
        anchors.bottom: controls_bg.top
        height: Constants.PlayIndicatorHeight

        source: "image://svgElement/:/SpotifyAssets.svg/playIndicatorBg"

        // Filled bar
        Rectangle
        {
            id: play_indicator_fill
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: (spotifySession.currentTrack) ? parent.width * (spotifySession.currentTrackPosition / spotifySession.currentTrack.durationMs) : 0
            color: Constants.SpotifyGreen
        }
    }

    // Album art
    Image
    {
        id: album_art
        anchors.bottom: play_indicator_bg.top
        anchors.top: parent.top
        anchors.left: parent.left
        width: Constants.AlbumArtWidth
        anchors.topMargin: Constants.StandardMargin
        anchors.bottomMargin: Constants.StandardMargin

        // Shown when no album art is available (yet)
        source: "image://svgElement/:/SpotifyAssets.svg/iconNoAlbumArt"

        sourceSize.height: Constants.AlbumArtWidth
        sourceSize.width: Constants.AlbumArtWidth

        // The actual album art
        Image
        {
            anchors.fill: parent

            fillMode: Image.PreserveAspectFit

            source: (spotifySession.currentTrack && spotifySession.currentTrack.albumCoverId.length > 0) ?
                        "image://spotify/" + spotifySession.currentTrack.albumCoverId : ""

            asynchronous: true
        }
    }

    // Play queue index
    Text
    {
        anchors.top: parent.top
        anchors.left: album_art.right
        anchors.margins: Constants.StandardMargin

        font.family: Constants.StandardFont
        font.pixelSize: 40
        font.bold: true
        color: Constants.DimmedTextColor

        text : queueText()

        function queueText()
        {
            if (spotifySession.radioMode)
            {
                //% "QTN_PLAYING_RADIO"
                return qsTrId("QTN_PLAYING_RADIO")
            }
            else
            {
                if (spotifySession.hasCurrentTrack)
                {
                    //% "QTN_PLAYING_TRACK_NUM"
                    return qsTrId("QTN_PLAYING_TRACK_NUM").arg(spotifySession.playQueue.currentIndex + 1).arg(spotifySession.playQueue.tracks.length)
                }
                else
                {
                    return ""
                }
            }
        }
    }

    // Shuffle button
    MouseArea
    {
        id: shuffle_button
        width: Constants.ShuffleButtonWidth
        height: Constants.ShuffleButtonHeight
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: Constants.StandardMargin
        anchors.bottomMargin: Constants.StandardMargin
        anchors.topMargin: Constants.StandardMargin

        visible: spotifySession.radioMode ? false : true

        Image
        {
            id: shuffle_image
            anchors.fill: parent

            sourceSize.height: parent.height
            sourceSize.width: parent.width

            source: (spotifySession.shuffle) ? "image://svgElement/:/SpotifyAssets.svg/btnSpotifyShuffle-toggleOn" :
                                               "image://svgElement/:/SpotifyAssets.svg/btnSpotifyShuffle-toggleOff"
        }

        onClicked: // Toggle shuffling
                   if (spotifySession.shuffle == false)
                   {
                       spotifySession.shuffle = true
                   }
                   else
                   {
                       spotifySession.shuffle = false
                   }
    }

    // Artist, album and track names
    Item
    {
        id: song_infos

        anchors.right: parent.right
        anchors.left: album_art.right
        anchors.bottom: play_indicator_bg.top
        anchors.top: shuffle_button.bottom

        // Artist
        Text
        {
            id: artist_text
            anchors.top: parent.top
            anchors.margins: Constants.StandardMargin
            anchors.left: parent.left
            anchors.right: parent.right

            font.family: Constants.StandardFont
            font.pixelSize: 40
            font.bold: true
            font.capitalization: spotifySession.currentTrack ? Font.AllUppercase : Font.MixedCase
            color: "white"
            elide: Text.ElideRight

            //% "QTN_PLAYING_NO_TRACK"
            text : spotifySession.currentTrack ? spotifySession.currentTrack.artists : qsTrId("QTN_PLAYING_NO_TRACK")
        }

        // Album
        Text
        {
            id: album_text
            anchors.top: artist_text.bottom
            anchors.margins: Constants.StandardMargin
            anchors.right: parent.right
            anchors.left: parent.left

            font.family: Constants.StandardFont
            font.pixelSize: 38
            font.bold: true
            color: Constants.SpotifyGreen
            elide: Text.ElideRight

            text : spotifySession.currentTrack ? spotifySession.currentTrack.album : ""
        }

        // Song
        Text
        {
            id: song_text
            anchors.top: album_text.bottom
            anchors.margins: Constants.StandardMargin
            anchors.right: parent.right
            anchors.left: parent.left

            font.family: Constants.StandardFont
            font.pixelSize: 40
            font.bold: true
            color: Constants.DimmedTextColor
            elide: Text.ElideRight

            text : spotifySession.currentTrack ? spotifySession.currentTrack.name : ""
        }
    }
}
