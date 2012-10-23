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
    id: car_mode_list

    signal listItemClicked(string item)
    signal optionsDone // Options should be closed (completely)

    property variant listModel

    property string type: "normal" // Can be "normal" and "radio"

    property int selected_index // Used in radio button lists

    property variant backStepTo: null

    onBackClicked: root_window.pageStack.pop(backStepTo, true)

    // Scroll up button
    SVGImageButton
    {
        id: scroll_up_btn

        anchors.top: parent.top
        anchors.right: parent.right
        width: Constants.NaviButtonWidth
        height: parent.height/2

        icon: (list.atYBeginning || (list.getHeightRatio() >= 1.0)) ? "SpotifyAssets.svg/iconScrollUp-disabled" : "SpotifyAssets.svg/iconScrollUp-normal"

        background: "SpotifyAssets.svg/btnBgBackRight-normal"

        enabled: (list.atYBeginning || (list.getHeightRatio() >= 1.0)) ? false : true

        onClicked:
        {
            if (!list.atYBeginning && (list.getHeightRatio() < 1.0))
            {
                // Scroll list up

                // Find the current element shown at the top of the list
                var current_pos = Math.round(car_mode_list.listModel.count * list.visibleArea.yPosition);

                var scroll_to = current_pos - Constants.ListScrollBy
                if (scroll_to < 0)
                {
                    // Can't scroll to negative indeces
                    scroll_to = 0
                }

                list.positionViewAtIndex(scroll_to, ListView.Beginning);
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

        icon: (list.atYEnd || (list.getHeightRatio() >= 1.0)) ? "SpotifyAssets.svg/iconScrollDown-disabled" : "SpotifyAssets.svg/iconScrollDown-normal"

        background: "SpotifyAssets.svg/btnBgBackRight-normal"

        enabled: (list.atYEnd || (list.getHeightRatio() >= 1.0)) ? false : true

        onClicked:
        {
            if (!list.atYEnd && (list.getHeightRatio() < 1.0))
            {
                // Scroll list down

                // Find the current element shown at the top of the list
                var current_pos = Math.round(car_mode_list.listModel.count * list.visibleArea.yPosition);

                list.positionViewAtIndex(current_pos + Constants.ListScrollBy, ListView.Beginning);
            }
        }
    }

    // Scroll bar
    Rectangle
    {
        id: scrollbar_bg

        anchors.right: scroll_up_btn.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: Constants.ScrollbarWidth

        color: Constants.ScrollbarBGColor
        radius: Constants.ScrollbarRadius

        anchors.rightMargin: Constants.ScrollbarMargin

        // Highlight
        Rectangle
        {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            height: calcHeight()

            function calcHeight()
            {
                var height = parent.height * list.getHeightRatio()

                // When scrolling past the boundarys we shrink the size
                if (list.visibleArea.yPosition < 0)
                {
                    height = Math.max(height + (list.visibleArea.yPosition * height), 0)
                }
                else if (anchors.topMargin + height > parent.height)
                {
                    height = Math.max(parent.height - anchors.topMargin, 0)
                }

                return height
            }

            radius: parent.radius
            color: Constants.ScrollbarFGColor

            // Position is controlled by setting the margin
            anchors.topMargin: Math.max(list.visibleArea.yPosition, 0) * parent.height

            // Hide the highlight if the entire list is in view
            visible: (list.getHeightRatio() >= 1.0) ? false : true

        }
    }

    // List of options
    ListView
    {
        id: list

        anchors.right: scrollbar_bg.left
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        model: car_mode_list.listModel

        clip: true

        // Only allow the user to scroll using the buttons
        interactive: false

        function getHeightRatio()
        {
            var height_ratio = list.visibleArea.heightRatio

            if (height_ratio == 0)
            {
                // There's a bug where height ratio is 0 until the user drags the first time... HACK to get around this
                height_ratio = max(list.height / (car_mode_list.listModel.count * Constants.ListElementHeight), 0)
                console.log("Height ratio corrected to " + height_ratio)
            }

            return height_ratio
        }

        delegate: CarModeListItem {
            model_count: list.model.count
            list_type: car_mode_list.type
            selected_index: car_mode_list.selected_index
            onClicked: car_mode_list.listItemClicked(model.identifier)
        }
    }

    // Hide top separator
    Rectangle
    {
        visible: (list.visibleArea.yPosition == 0) ? false : true

        height: Constants.ListSeparatorHeight / 2
        anchors.right: scrollbar_bg.left
        anchors.left: parent.left
        anchors.top: parent.top

        color: "black"
    }

    // Hide bottom separator
    Rectangle
    {
        height: Constants.ListSeparatorHeight / 2
        anchors.right: scrollbar_bg.left
        anchors.left: parent.left
        anchors.bottom: parent.bottom

        color: "black"
    }
}
