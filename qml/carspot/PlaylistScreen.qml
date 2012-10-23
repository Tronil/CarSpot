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
import carspot 1.0

CarModeList
{
    id: playlist_list

    listModel: PlaylistModel { id: model }

    property int playlist_index

    // Set the playlist property here to avoid a binding (which can change the playlist if there is a connection issue,
    // since the playlists may change position)
    Component.onCompleted: model.playlist = spotifySession.user.playlists[playlist_list.playlist_index]

    onListItemClicked:
    {
        if (item == 0)
        {
            // Play item
            spotifySession.user.playlists[playlist_list.playlist_index].play()
            optionsDone()
        }
        else if (item == 1)
        {
            // Toggle offline item
            spotifySession.user.playlists[playlist_list.playlist_index].availableOffline = !spotifySession.user.playlists[playlist_list.playlist_index].availableOffline
        }
        else
        {
            // Tracks - play the track if possible
            var track = spotifySession.user.playlists[playlist_list.playlist_index].tracks[item - 2]

            if (!spotifySession.offlineMode || track.offlineStatus == SpotifyTrack.Yes)
            {
                track.play()
                optionsDone()
            }
        }
    }
}
