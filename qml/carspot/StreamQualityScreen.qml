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
    listModel: ListModel {

        // High quality
        ListElement
        {
            identifier: "High"
            text: ""
            sublist: false
        }

        // Low quality
        ListElement
        {
            identifier: "Low"
            text: ""
            sublist: false
        }

        Component.onCompleted: translate()

        function translate()
        {
            var texts = [
                        //% "QTN_LIST_HIGH_BW"
                        QT_TRID_NOOP("QTN_LIST_HIGH_BW"),
                        //% "QTN_LIST_LOW_BW"
                        QT_TRID_NOOP("QTN_LIST_LOW_BW"),
                    ]
            var i = 0
            while (i < count)
            {
                setProperty(i, "text", qsTrId(texts[i]))
                i++
            }
        }
    }

    type: "radio"

    selected_index: (spotifySession.streamingQuality == SpotifySession.HighQuality) ? 0 : 1

    onListItemClicked:
    {
        if (item == "High")
        {
            spotifySession.streamingQuality = SpotifySession.HighQuality
        }
        else
        {
            spotifySession.streamingQuality = SpotifySession.LowQuality
        }

        backClicked()
    }
}

