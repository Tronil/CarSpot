# Add more folders to ship with the application, here
folder_01.source = qml/carspot
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

libspotify.source = libspotify/lib
libspotify.target =
DEPLOYMENTFOLDERS += libspotify

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

symbian:TARGET.UID3 = 0xE2DC2D12

# Smart Installer package's UID
# This UID is from the protected range and therefore the package will
# fail to install if self-signed. By default qmake uses the unprotected
# range value if unprotected UID is defined for the application and
# 0x2002CCCF value if protected UID is given to the application
#symbian:DEPLOYMENT.installer_header = 0x2002CCCF

# Allow network access on Symbian
symbian:TARGET.CAPABILITY += NetworkServices

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
# CONFIG += mobility
# MOBILITY +=

# Speed up launching on MeeGo/Harmattan when using applauncherd daemon
CONFIG += qdeclarative-boostable

SOURCES += main.cpp \
    svgElementImageProvider.cpp \
    NetworkAccess.cpp \
    CarModeAppCaller.cpp \
    PlaylistModel.cpp \
    PlaylistsModel.cpp \
    Constants.cpp \
    KeyEvents.cpp \
    WhatsNewModel.cpp \
    NewAlbumModel.cpp \
    ListItem.cpp \
    AddToPlaylistModel.cpp \
    SingleInstanceInsurer.cpp \
    UnixSignalHandler.cpp

# lupdate only looks at files specfied in SOURCES, but apparently doesn't care about
# conditionals, so we add the QML files in a conditional that is never true
hack_for_lupdate {
SOURCES += \
    qml/carspot/AboutScreen.qml \
    qml/carspot/AddToPlaylistScreen.qml \
    qml/carspot/CarModeList.qml \
    qml/carspot/CarModeListItem.qml \
    qml/carspot/CarModePage.qml \
    qml/carspot/CarNote.qml \
    qml/carspot/HWKeys.qml \
    qml/carspot/LoginScreen.qml \
    qml/carspot/main.qml \
    qml/carspot/NewAlbumScreen.qml \
    qml/carspot/OfflineSyncScreen.qml \
    qml/carspot/OptionsModel.qml \
    qml/carspot/OptionsScreen.qml \
    qml/carspot/PlayingScreen.qml \
    qml/carspot/PlaylistScreen.qml \
    qml/carspot/PlaylistsScreen.qml \
    qml/carspot/RadioScreen.qml \
    qml/carspot/SettingsModel.qml \
    qml/carspot/SettingsScreen.qml \
    qml/carspot/StreamQualityScreen.qml \
    qml/carspot/SVGImageButton.qml \
    qml/carspot/WaitAnimation.qml \
    qml/carspot/WelcomeScreen.qml \
    qml/carspot/WhatsNewScreen.qml
}

CONFIG += qmsystem2

LIBS += -lQtSvg

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/manifest.aegis \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog \
    qtc_packaging/debian_harmattan/postrm \
    libQtSpotify/libQtSpotify.pri

HEADERS += \
    svgElementImageProvider.hpp \
    NetworkAccess.hpp \
    CarModeAppCaller.hpp \
    PlaylistModel.hpp \
    PlaylistsModel.hpp \
    Constants.hpp \
    KeyEvents.hpp \
    WhatsNewModel.hpp \
    NewAlbumModel.hpp \
    ListItem.hpp \
    AddToPlaylistModel.hpp \
    SingleInstanceInsurer.hpp \
    UnixSignalHandler.hpp

RESOURCES += \
    carspot.qrc

include(libQtSpotify/libQtSpotify.pri)

# Quiet the "the mangling of 'va_list' has changed in GCC 4.4" warning
QMAKE_CXXFLAGS += -Wno-psabi

# Add the path to the spotify library to the shared libraries search path
QMAKE_LFLAGS += -Wl,-rpath,/opt/carspot/lib

DEFINES += QT_NO_CAST_FROM_ASCII

include(carspot.pri)

TRANSLATIONS = localization/carspot_en.ts
