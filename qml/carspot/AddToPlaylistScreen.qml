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
    id: add_to_playlist_list

    listModel: AddToPlaylistModel {}

    property variant albumBrowser

    property string newPlaylistName: ""

    onListItemClicked:
    {
        if (item == 0)
        {
            // Add as new playlist
            newPlaylistName = albumBrowser.album.artist + " | " + albumBrowser.album.name

            spotifySession.user.createPlaylist(newPlaylistName)
            // Wait for the new playlist to be created
        }
        else
        {
            // Add to existing playlist
            var playlist = add_to_playlist_list.listModel.playlists[item-1]
            playlist.addAlbum(albumBrowser)

            //% "QTN_NOTE_ADDED_TO_PLAYLIST"
            root_window.showNote(qsTrId("QTN_NOTE_ADDED_TO_PLAYLIST").arg(playlist.name))

            backClicked()
        }
    }

    Connections
    {
        target: spotifySession.user

        onPlaylistsChanged:
        {
            if (add_to_playlist_list.newPlaylistName != "")
            {
                // New playlist created
                var name = add_to_playlist_list.newPlaylistName
                add_to_playlist_list.newPlaylistName = ""

                // find the new playlist
                var playlists = spotifySession.user.playlists

                // The first two playlists are inbox and starred so skip those
                var i = 2
                while (i < playlists.length && playlists[i].name != name)
                {
                    i++;
                }

                if (i < playlists.length)
                {
                    // Add the album
                    playlists[i].addAlbum(albumBrowser)
                }
                else
                {
                    console.log("Playlist not found!")
                }

                //% "QTN_NOTE_ADDED_AS_NEW_PLAYLIST"
                root_window.showNote(qsTrId("QTN_NOTE_ADDED_AS_NEW_PLAYLIST"))

                backClicked()
            }
        }
    }
}
