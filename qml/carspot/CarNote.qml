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
/*
  Simple error note component. Note that this component will call destroy on itself, so it should
  be created with createObject()
*/

import QtQuick 1.1
import com.nokia.meego 1.1     // MeeGo 1.2 Harmattan components

Rectangle
{
   id: error_note

   anchors.fill: parent

   color: "black"

   property string text: ""

   Item
   {
       anchors.top: parent.top
       anchors.bottom: error_button.top
       anchors.left: parent.left
       anchors.right: parent.right
       anchors.margins: Constants.ErrorTextMargins

       Text
       {
           anchors.centerIn: parent

           width: parent.width
           font.pixelSize: 36
           font.family: Constants.ErrorFont
           color: "white"
           horizontalAlignment: Text.AlignHCenter

           wrapMode: Text.Wrap

           text: error_note.text
       }
   }

   // Button
   MouseArea
   {
       id: error_button

       anchors.bottom: parent.bottom
       anchors.bottomMargin: Constants.ErrorButtonBottomOffset
       anchors.horizontalCenter: parent.horizontalCenter

       width: Constants.ErrorButtonWidth
       height: Constants.ErrorButtonHeight

       onClicked: parent.destroy()

       // Background
       Image
       {
           anchors.fill: parent

           source: parent.pressed ? "image://svgElement/:/SpotifyAssets.svg/btnBgBigBasic-pressed" : "image://svgElement/:/SpotifyAssets.svg/btnBgBigBasic-normal"
       }

       // Button text
       Text
       {
           anchors.centerIn: parent

           font.family: Constants.ErrorFont
           font.pixelSize: 30
           color: "white"

           //% "QTN_BTN_OK"
           text: qsTrId("QTN_BTN_OK")
       }
   }
}
