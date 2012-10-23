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
    // Force offline mode
    ListElement
    {
        identifier: "Offline"
        text: ""
        second_row_text: ""
        toggle_item: true
        toggle_state: false // The correct state has to be set by javascript
        sublist: false
    }

    // Streaming quality / bandwidth
    ListElement
    {
        identifier: "Stream quality"
        text: ""
        second_row_text: "" // The correct text has to be set by javascript
        toggle_item: false
        toggle_state: false
        sublist: true
    }

    // Offline sync quality
    ListElement
    {
        identifier: "Offline sync"
        text: ""
        second_row_text: "" // The correct text has to be set by javascript
        toggle_item: false
        toggle_state: false
        sublist: true
    }

    // Sync over mobile connections
    ListElement
    {
        identifier: "Sync over mobile"
        text: ""
        second_row_text: ""
        toggle_item: true
        toggle_state: false // The correct state has to be set by javascript
        sublist: false
    }

    // Log out the current user
    ListElement
    {
        identifier: "Logout"
        text: ""
        second_row_text: ""
        toggle_item: false
        toggle_state: false
        sublist: false
    }

    // Show about screen
    ListElement
    {
        identifier: "About"
        text: ""
        second_row_text: ""
        toggle_item: false
        toggle_state: false
        sublist: false
    }

    Component.onCompleted: translate()

    // Scripts are not allowed in list elements AND there is a bug in QML which causes QT_TRID_NOOP
    // to be regarded as a script; work around this in this function
    function translate()
    {
        var texts = [
                    //% "QTN_LIST_OFFLINE_MODE"
                    QT_TRID_NOOP("QTN_LIST_OFFLINE_MODE"),
                    //% "QTN_LIST_STREAMING"
                    QT_TRID_NOOP("QTN_LIST_STREAMING"),
                    //% "QTN_LIST_OFFLINE_SYNC"
                    QT_TRID_NOOP("QTN_LIST_OFFLINE_SYNC"),
                    //% "QTN_LIST_SYNC_OVER_MOBILE"
                    QT_TRID_NOOP("QTN_LIST_SYNC_OVER_MOBILE"),
                    //% "QTN_LIST_LOGOUT"
                    QT_TRID_NOOP("QTN_LIST_LOGOUT"),
                    //% "QTN_LIST_ABOUT"
                    QT_TRID_NOOP("QTN_LIST_ABOUT")
                ]
        var i = 0
        while (i < count)
        {
            setProperty(i, "text", qsTrId(texts[i]))
            i++
        }
    }
}
