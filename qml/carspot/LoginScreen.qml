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

CarModePage
{
    id: login_screen

    signal loginDone

    disableBackButton: true

    Column
    {
        id: column

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: Constants.StandardMargin

        Label
        {
            id: login_text

            anchors.horizontalCenter: parent.horizontalCenter

            //% "QTN_LOGIN_SPOTIFY"
            text: qsTrId("QTN_LOGIN_SPOTIFY")
            font.family: Constants.StandardFont
            font.pixelSize: 42
            font.bold: true
            color: "white"
        }

        // User name
        TextField
        {
            id: username

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Constants.StandardMargin

            //% "QTN_USER_NAME"
            placeholderText: qsTrId("QTN_USER_NAME")
            font.family: Constants.StandardFont
            font.pixelSize: 38

            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText

            platformSipAttributes: SipAttributes { actionKeyEnabled: true }

            Keys.onReturnPressed: password.forceActiveFocus()
        }

        // Password
        TextField
        {
            id: password

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Constants.StandardMargin

            //% "QTN_PASSWORD"
            placeholderText: qsTrId("QTN_PASSWORD")
            font.family: Constants.StandardFont
            font.pixelSize: 38
            echoMode: TextInput.Password

            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText

            platformSipAttributes: SipAttributes { actionKeyEnabled: true }

            Keys.onReturnPressed:
            {
                login_text.focus = true // To close the virtual keyboard

                if (username.text.length > 0 && password.text.length > 0)
                    spotifySession.login(username.text, password.text)
            }
        }


        // Login button
        Button
        {
            id: login_button

            anchors.left: parent.left
            anchors.margins: Constants.StandardMargin

            //% "QTN_BTN_LOGIN"
            text: qsTrId("QTN_BTN_LOGIN")
            enabled: username.text.length > 0 && password.text.length > 0

            onClicked: spotifySession.login(username.text, password.text)

            platformStyle: ButtonStyle { background: "image://svgElement/:/SpotifyAssets.svg/btnBgSpotifyLogin-normal_1_"
                                         disabledBackground: "image://svgElement/:/SpotifyAssets.svg/btnBgSpotifyLogin-normal_1_"
                                         pressedBackground: "image://svgElement/:/SpotifyAssets.svg/btnBgSpotifyLogin-pressed"
                                         fontFamily: Constants.StandardFont
                                         fontPixelSize: 38
                                         textColor: "white"
                                         buttonHeight: 100
                                         buttonWidth: 430 }
        }
    }

    // Spotify privacy policy
    Label
    {
        id: privacy_policy

        anchors.topMargin: 2*Constants.StandardMargin
        anchors.top: column.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: Constants.StandardMargin

        color: "white"
        font.pixelSize: 38
        wrapMode: Text.WordWrap

        //% "QTN_PRIVACY_POLICY"
        text: "<style type=text/css> a {text-decoration: underline; color:"+Constants.SpotifyGreen+"} </style>" + qsTrId("QTN_PRIVACY_POLICY")

        onLinkActivated: Qt.openUrlExternally(link)
    }

    WaitAnimation
    {
        id: wait_anim

        anchors.fill: parent
        visible: false
    }

    // Logic
    Connections
    {
        target: spotifySession

        onLoggingIn:
        {
            wait_anim.visible = true
        }

        onConnectionErrorChanged:
        {

            wait_anim.visible = false
        }

        onIsLoggedInChanged:
        {
            wait_anim.visible = false

            if (spotifySession.isLoggedIn)
            {
                loginDone()
            }
        }
    }
}
