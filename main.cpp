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
#include <iostream>

#include <QtGui/QApplication>
#include "qmlapplicationviewer.h"
#include "svgElementImageProvider.hpp"
#include <QDeclarativeEngine>
#include "libQtSpotify/QtSpotify"
#include "libQtSpotify/qspotify_qmlplugin.h"
#include <QDeclarativeContext>
#include "NetworkAccess.hpp"
#include "CarModeAppCaller.hpp"
#include "PlaylistModel.hpp"
#include "PlaylistsModel.hpp"
#include "Constants.hpp"
#include "KeyEvents.hpp"
#include "WhatsNewModel.hpp"
#include "NewAlbumModel.hpp"
#include "AddToPlaylistModel.hpp"
#include "SingleInstanceInsurer.hpp"
#include "UnixSignalHandler.hpp"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    SingleInstanceInsurer singleInstance; // Will block until others are killed
    QScopedPointer<QApplication> app(createApplication(argc, argv));
    QmlApplicationViewer viewer;
    QDeclarativeEngine *engine = viewer.engine();
    QString locale = QLocale::system().name();
    QTranslator translator;

    // POSIX/Unix Signal handler
    UnixSignalHandler signalHandler;
    app->connect(&signalHandler, SIGNAL(SigInt()), app.data(), SLOT(quit()));

    QApplication::setOrganizationName(QLatin1String("Nokia"));
    QApplication::setApplicationName(QLatin1String("carspot"));

    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QLatin1String("/home/user/MyDocs/.carspot"));

    if (!translator.load(QLatin1String("carspot_") + locale, QLatin1String(":/localization")))
    {
        // Fall back to english
        translator.load(QLatin1String("carspot_en"), QLatin1String(":/localization"));
    }
    app->installTranslator(&translator);

    // Register Spotify QML components
    registerQmlTypes();

    // Used for getting a single element from a SVG file
    engine->addImageProvider(QLatin1String("svgElement"), new svgElementImageProvider);

    // For getting album art
    engine->addImageProvider(QLatin1String("spotify"), new QSpotifyImageProvider);

    // Access to spotify session object
    QScopedPointer<QSpotifySession> spotifySession(QSpotifySession::instance());
    viewer.rootContext()->setContextProperty(QLatin1String("spotifySession"), spotifySession.data());

    // Access to car mode application (via DBus)
    CarModeAppCaller carModeCaller;
    viewer.rootContext()->setContextProperty(QLatin1String("carmodeApp"), &carModeCaller);

    // Listen to HW key presses
    KeyEvents keyEvents;
    viewer.rootContext()->setContextProperty(QLatin1String("keyEvents"), &keyEvents);
    qmlRegisterUncreatableType<KeyEvents>(QLatin1String("KeyEvents").latin1(), 1, 0, "KeyEvents", QLatin1String("Use the Context property instead"));

    // Constants
    Constants constants;
    viewer.rootContext()->setContextProperty(QLatin1String("Constants"), &constants);

    // List model plugins
    qmlRegisterType<PlaylistModel>("carspot", 1, 0, "PlaylistModel");
    qmlRegisterType<PlaylistsModel>("carspot", 1, 0, "PlaylistsModel");
    qmlRegisterType<WhatsNewModel>("carspot", 1, 0, "WhatsNewModel");
    qmlRegisterType<NewAlbumModel>("carspot", 1, 0, "NewAlbumModel");
    qmlRegisterType<AddToPlaylistModel>("carspot", 1, 0, "AddToPlaylistModel");

    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/carspot/main.qml"));
    viewer.showExpanded();

    NetworkAccess network; // (Attempt to) open a network connection

    return app->exec();
}
