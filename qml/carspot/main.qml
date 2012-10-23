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

// Root element that contains the entire screen
PageStackWindow
{
    id: root_window

    // N9 resolution is 854 x 480 (FWVGA)

    initialPage: PlayingScreen {}

    showStatusBar: false
    showToolBar: false

    property variant page_to_push: null

    signal loginDone

    signal openOptions

    signal optionsDone

    signal isLoggedInChanged

    Component.onCompleted:
    {
        // Repeat should always be on
        spotifySession.repeat = true

        if (!spotifySession.isLoggedIn)
        {
            showLoginScreen()
        }

        spotifySession.isLoggedInChanged.connect(root_window.isLoggedInChanged)

        initialPage.openOptions.connect(root_window.openOptions)
    }

    onLoginDone:
    {
        // Close login screen
        pageStack.pop(null, true)

        // Show welcome screen
        var component = Qt.createComponent("WelcomeScreen.qml");
        var welcome = component.createObject(root_window);

        if (welcome == null)
        {
            console.log("Error creating object");
        }
    }

    onOpenOptions:
    {
        var new_page = pageStack.push(Qt.resolvedUrl("OptionsScreen.qml"), {}, true)

        new_page.optionsDone.connect(root_window.optionsDone)
    }

    onOptionsDone:
    {
        // Close all open pages except PlayingScreen
        pageStack.pop(null, true)
    }

    onIsLoggedInChanged:
    {
        if (!spotifySession.isLoggedIn)
        {
            // User logged out
            showLoginScreen()
        }
    }

    function showLoginScreen()
    {
        var new_page = pageStack.push(Qt.resolvedUrl("LoginScreen.qml"), {}, true)
        new_page.loginDone.connect(root_window.loginDone)
    }

    Connections
    {
        target: spotifySession

        onConnectionErrorChanged:
        {
            if (spotifySession.connectionError != spotifySession.ConnectionOk)
            {
                showNote(spotifySession.connectionErrorMessage)
            }
        }

        onOfflineErrorChanged:
        {
            if (spotifySession.offlineError != spotifySession.OfflineOk)
            {
                showNote(spotifySession.offlineErrorMessage)
            }
        }
    }

    function showNote(text)
    {
        console.log(text)

        // Show a popup with the error message
        var component = Qt.createComponent("CarNote.qml");
        var login = component.createObject(root_window, { "text" : text});

        if (login == null)
        {
            console.log("Error creating object");
        }
    }
}

