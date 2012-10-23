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

CarModePage
{
    property variant backStepTo

    onBackClicked: root_window.pageStack.pop(backStepTo, true)

    // Content
    Flickable
    {
        id: content
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: scroll_up_btn.left
        anchors.bottom: parent.bottom

        contentHeight: about_headline.height + about_text.height
        contentWidth: width

        interactive: false

        // Headline
        Text
        {
            id: about_headline

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top

            anchors.topMargin: Constants.StandardMargin

            color: "white"

            font.family: Constants.StandardFont
            font.bold: true
            font.pixelSize: 49

            text: "CarSpot " + Constants.Version
        }

        // Main text
        Text
        {
            id: about_text

            anchors.top: about_headline.bottom
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.topMargin: 2*Constants.StandardMargin
            anchors.leftMargin: Constants.StandardMargin
            anchors.rightMargin: Constants.StandardMargin

            color: "white"

            font.family: Constants.StandardFont
            font.pixelSize: 38

            textFormat: Text.RichText
            wrapMode: Text.Wrap

            text: //% "QTN_ABOUT_COPYRIGHT"
                  "<p>" + qsTrId("QTN_ABOUT_COPYRIGHT") + "</p>"
                  //% "QTN_ABOUT_LIBSPOTIFY"
                  + "<p>" + qsTrId("QTN_ABOUT_LIBSPOTIFY") + "</p>"
                  //% "QTN_ABOUT_MEESPOT"
                  + "<p>" + qsTrId("QTN_ABOUT_MEESPOT") + "</p>"
        }
    }

    // Scroll up button
    SVGImageButton
    {
        id: scroll_up_btn

        anchors.top: parent.top
        anchors.right: parent.right
        width: Constants.NaviButtonWidth
        height: parent.height/2

        icon: (content.atYBeginning) ? "SpotifyAssets.svg/iconScrollUp-disabled" : "SpotifyAssets.svg/iconScrollUp-normal"

        background: "SpotifyAssets.svg/btnBgBackRight-normal"

        enabled: (content.atYBeginning) ? false : true

        onClicked:
        {
            if (!content.atYBeginning)
            {
                // Scroll list up, with some overlap
                content.contentY = Math.max(content.contentY - (content.height - about_text.font.pixelSize*2), 0)
            }
        }
    }

    // Scroll down button
    SVGImageButton
    {
        id: scroll_down_btn

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.top: scroll_up_btn.bottom
        anchors.left: scroll_up_btn.left

        icon: (content.atYEnd) ? "SpotifyAssets.svg/iconScrollDown-disabled" : "SpotifyAssets.svg/iconScrollDown-normal"

        background: "SpotifyAssets.svg/btnBgBackRight-normal"

        enabled: (content.atYEnd) ? false : true

        onClicked:
        {
            if (!content.atYEnd)
            {
                // Scroll list down, with some overlap
                content.contentY = content.contentY + (content.height - about_text.font.pixelSize*2)
            }
        }
    }

}
