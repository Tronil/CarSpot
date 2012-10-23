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

MouseArea
{
    width: parent.width
    height: Constants.ListElementHeight

    property int model_count

    property bool two_row_item: (model.second_row_text && model.second_row_text != "") ? true : false

    property string list_type: "normal"

    property int selected_index

    // Background
    Image
    {
        anchors.fill: parent

        source: (model.background) ? model.background : ""

        // Pressed color
        Rectangle
        {
            anchors.fill: parent
            color: Constants.SpotifyGreen
            visible: (parent.parent.pressed) ? true : false
        }
    }

    // Icon
    Image
    {
        id: item_icon

        width: Constants.ListIconWidth
        height: Constants.ListIconHeight
        sourceSize.width: width
        sourceSize.height: height

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: Constants.ListIconMargin

        source: (model.icon) ? model.icon : ""
    }

    // Text
    Text
    {
        id: item_text

        anchors.right: if (model.sublist)
                           sublist_icon.left
                       else if (model.toggle_item)
                           toggle_icon.left
                       else
                           parent.right
        anchors.left: (model.icon) ? item_icon.right : parent.left
        anchors.verticalCenter: parent.verticalCenter

        anchors.leftMargin: (model.icon) ? Constants.ListIconMargin : Constants.StandardMargin
        anchors.rightMargin: Constants.ListIconMargin

        elide: Text.ElideRight

        color: (model.dimmed && model.dimmed == true) ? Constants.DimmedTextColor : "white"
        text: model.text

        font.family: Constants.StandardFont
        font.pixelSize: Constants.ListMainTextSize

        visible: (parent.two_row_item) ? false : true
    }


    Item
    {
        // Only used in 2 row text items
        visible: parent.two_row_item

        anchors.right: if (model.sublist)
                           sublist_icon.left
                       else if (model.toggle_item)
                           toggle_icon.left
                       else
                           parent.right
        anchors.left: (model.icon) ? item_icon.right : parent.left
        anchors.verticalCenter: parent.verticalCenter

        anchors.leftMargin: (model.icon) ? Constants.ListIconMargin : Constants.StandardMargin
        anchors.rightMargin: Constants.ListIconMargin

        height: item_1st_row_text.implicitHeight + item_2nd_row_text.implicitHeight

        // 1st row text
        Text
        {
            id: item_1st_row_text

            anchors.left: parent.left
            anchors.right: parent.right

            elide: Text.ElideRight

            color: (model.dimmed && model.dimmed == true) ? Constants.DimmedTextColor : "white"
            text: model.text

            font.family: Constants.StandardFont
            font.pixelSize: Constants.ListMainTextSize
        }

        // 2nd row text
        Text
        {
            id: item_2nd_row_text
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: item_1st_row_text.bottom

            elide: Text.ElideRight

            font.family: Constants.StandardFont
            font.pixelSize: Constants.ListSecondaryTextSize

            color: if (parent.parent.pressed)
                   {
                       if (!model.second_row_text_color || model.second_row_text_color == Constants.SpotifyGreen)
                           "white"
                       else
                           model.second_row_text_color
                   }
                   else
                       (model.second_row_text_color) ? model.second_row_text_color : Constants.SpotifyGreen

            text: (model.second_row_text) ? model.second_row_text : ""

        }
    }

    // Sublist icon - doubles as the radio button icon in radio button lists
    Image
    {
        id: sublist_icon

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: Constants.ListEndMargin

        width: (parent.list_type == "radio") ? Constants.RadioButtonWidth : Constants.SublistIconWidth
        height: (parent.list_type == "radio") ? Constants.RadioButtonHeight : Constants.SublistIconHeight
        sourceSize.width: width
        sourceSize.height: height

        source: if (parent.list_type == "radio")
                {
                    if (model.index == selected_index)
                        "image://svgElement/:/SpotifyAssets.svg/iconSpotifyRadiobtn-selected_1_"
                    else
                        "image://svgElement/:/SpotifyAssets.svg/iconSpotifyRadiobtn-normal"
                }
                else
                    "image://svgElement/:/SpotifyAssets.svg/rowArrow-selected"

        visible: (model.sublist || parent.list_type == "radio")
    }

    // Toggle icon
    Image
    {
        id: toggle_icon

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 22

        width: Constants.ToggleButtonWidth
        height: Constants.ToggleButtonHeight
        sourceSize.width: width
        sourceSize.height: height

        source: (model.toggle_state) ? "image://svgElement/:/SpotifyAssets.svg/btnSpotifyToggle-On" : "image://svgElement/:/SpotifyAssets.svg/btnSpotifyToggle-Off"

        visible: (model.toggle_item) ? true : false
    }

    // Separator line - half is shown on the bottom, half on the top (so it on top of the background)
    Rectangle
    {
        // Top part
        height: Constants.ListSeparatorHeight/2

        color: Constants.SpotifyGreen

        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: parent.top

        // Don't show on the first element in the list
        visible: (model.index == 0) ? false : true
    }

    Rectangle
    {
        // Bottom part
        height: Constants.ListSeparatorHeight/2

        color: Constants.SpotifyGreen

        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: parent.bottom

        // Don't show on the last element in the list
        visible: (model.index == parent.model_count - 1) ? false : true
    }
}
