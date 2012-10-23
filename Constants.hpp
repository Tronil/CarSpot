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
#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <QHash>

// Data roles (for use in list models)
enum Roles
{
    TextRole = Qt::UserRole+1,
    IdentifierRole,
    SecondRowTextRole,
    SecondRowTextColorRole,
    IconRole,
    SublistRole,
    ToggleItemRole,
    ToggleStateRole,
    BackgroundRole,
    DimmedRole
};

class Constants : public QObject
{
    Q_OBJECT
    // For QML compatibility everything has to be properties
    Q_PROPERTY(QString SpotifyGreen READ getSpotifyGreen CONSTANT FINAL)
    Q_PROPERTY(QString DimmedTextColor READ getDimmedTextColor CONSTANT FINAL)
    Q_PROPERTY(QString StandardFont READ getStandardFont CONSTANT FINAL)
    Q_PROPERTY(int NaviButtonWidth READ getNaviButtonWidth CONSTANT FINAL)
    Q_PROPERTY(int StandardMargin READ getStandardMargin CONSTANT FINAL)
    Q_PROPERTY(QString Version READ getVersion CONSTANT FINAL)
    Q_PROPERTY(int ControlsHeight READ getControlsHeight CONSTANT FINAL)
    Q_PROPERTY(int ControlWidth READ getControlWidth CONSTANT FINAL)
    Q_PROPERTY(int PlayIndicatorHeight READ getPlayIndicatorHeight CONSTANT FINAL)
    Q_PROPERTY(int AlbumArtWidth READ getAlbumArtWidth CONSTANT FINAL)
    Q_PROPERTY(int ShuffleButtonWidth READ getShuffleButtonWidth CONSTANT FINAL)
    Q_PROPERTY(int ShuffleButtonHeight READ getShuffleButtonHeight CONSTANT FINAL)
    Q_PROPERTY(int ListElementHeight READ getListElementHeight CONSTANT FINAL)
    Q_PROPERTY(int ListSeparatorHeight READ getListSeparatorHeight CONSTANT FINAL)
    Q_PROPERTY(int ListIconWidth READ getListIconWidth CONSTANT FINAL)
    Q_PROPERTY(int ListIconHeight READ getListIconHeight CONSTANT FINAL)
    Q_PROPERTY(int ListIconMargin READ getListIconMargin CONSTANT FINAL)
    Q_PROPERTY(int ListEndMargin READ getListEndMargin CONSTANT FINAL)
    Q_PROPERTY(int ListMainTextSize READ getListMainTextSize CONSTANT FINAL)
    Q_PROPERTY(int ListSecondaryTextSize READ getListSecondaryTextSize CONSTANT FINAL)
    Q_PROPERTY(int SublistIconHeight READ getSublistIconHeight CONSTANT FINAL)
    Q_PROPERTY(int SublistIconWidth READ getSublistIconWidth CONSTANT FINAL)
    Q_PROPERTY(int RadioButtonHeight READ getRadioButtonHeight CONSTANT FINAL)
    Q_PROPERTY(int RadioButtonWidth READ getRadioButtonWidth CONSTANT FINAL)
    Q_PROPERTY(int ToggleButtonHeight READ getToggleButtonHeight CONSTANT FINAL)
    Q_PROPERTY(int ToggleButtonWidth READ getToggleButtonWidth CONSTANT FINAL)
    Q_PROPERTY(int ListScrollBy READ getListScrollBy CONSTANT FINAL)
    Q_PROPERTY(int ScrollbarMargin READ getScrollbarMargin CONSTANT FINAL)
    Q_PROPERTY(int ScrollbarWidth READ getScrollbarWidth CONSTANT FINAL)
    Q_PROPERTY(int ScrollbarRadius READ getScrollbarRadius CONSTANT FINAL)
    Q_PROPERTY(QString ScrollbarBGColor READ getScrollbarBGColor CONSTANT FINAL)
    Q_PROPERTY(QString ScrollbarFGColor READ getScrollbarFGColor CONSTANT FINAL)
    Q_PROPERTY(int ErrorButtonHeight READ getErrorButtonHeight CONSTANT FINAL)
    Q_PROPERTY(int ErrorButtonWidth READ getErrorButtonWidth CONSTANT FINAL)
    Q_PROPERTY(int ErrorButtonBottomOffset READ getErrorButtonBottomOffset CONSTANT FINAL)
    Q_PROPERTY(int ErrorTextMargins READ getErrorTextMargins CONSTANT FINAL)
    Q_PROPERTY(QString ErrorFont READ getErrorFont CONSTANT FINAL)
    Q_PROPERTY(int WaitAnimationNumFrames READ getWaitAnimationNumFrames CONSTANT FINAL)
    Q_PROPERTY(int WaitAnimationInterval READ getWaitAnimationInterval CONSTANT FINAL)

public:
    // General stuff
    static const QString SpotifyGreen;
    static QString getSpotifyGreen() { return SpotifyGreen; }
    static const QString DimmedTextColor;
    static QString getDimmedTextColor() { return DimmedTextColor; }
    static const QString StandardFont;
    static QString getStandardFont() { return StandardFont; }
    static const int NaviButtonWidth;
    static int getNaviButtonWidth() { return NaviButtonWidth; }
    static const int StandardMargin;
    static int getStandardMargin() { return StandardMargin; }
    static const QString Version;
    static QString getVersion() { return Version; }

    // Playing screen
    static const int ControlsHeight;
    static int getControlsHeight() { return ControlsHeight; }
    static const int ControlWidth;
    static int getControlWidth() { return ControlWidth; }
    static const int PlayIndicatorHeight;
    static int getPlayIndicatorHeight() { return PlayIndicatorHeight; }
    static const int AlbumArtWidth;
    static int getAlbumArtWidth() { return AlbumArtWidth; }
    static const int ShuffleButtonWidth;
    static int getShuffleButtonWidth() { return ShuffleButtonWidth; }
    static const int ShuffleButtonHeight;
    static int getShuffleButtonHeight() { return ShuffleButtonHeight; }

    // Lists
    static const int ListElementHeight;
    static int getListElementHeight() { return ListElementHeight; }
    static const int ListSeparatorHeight;
    static int getListSeparatorHeight() { return ListSeparatorHeight; }
    static const int ListIconWidth;
    static int getListIconWidth() { return ListIconWidth; }
    static const int ListIconHeight;
    static int getListIconHeight() { return ListIconHeight; }
    static const int ListIconMargin;
    static int getListIconMargin() { return ListIconMargin; }
    static const int ListEndMargin;
    static int getListEndMargin() { return ListEndMargin; }
    static const int ListMainTextSize;
    static int getListMainTextSize() { return ListMainTextSize; }
    static const int ListSecondaryTextSize;
    static int getListSecondaryTextSize() { return ListSecondaryTextSize; }
    static const int SublistIconHeight;
    static int getSublistIconHeight() { return SublistIconHeight; }
    static const int SublistIconWidth;
    static int getSublistIconWidth() { return SublistIconWidth; }
    static const int RadioButtonHeight;
    static int getRadioButtonHeight() { return RadioButtonHeight; }
    static const int RadioButtonWidth;
    static int getRadioButtonWidth() { return RadioButtonWidth; }
    static const int ToggleButtonHeight;
    static int getToggleButtonHeight() { return ToggleButtonHeight; }
    static const int ToggleButtonWidth;
    static int getToggleButtonWidth() { return ToggleButtonWidth; }
    static const int ListScrollBy;
    static int getListScrollBy() { return ListScrollBy; }

    // Scrollbar appearence
    static const int ScrollbarMargin;
    static int getScrollbarMargin() { return ScrollbarMargin; }
    static const int ScrollbarWidth;
    static int getScrollbarWidth() { return ScrollbarWidth; }
    static const int ScrollbarRadius;
    static int getScrollbarRadius() { return ScrollbarRadius; }
    static const QString ScrollbarBGColor;
    static QString getScrollbarBGColor() { return ScrollbarBGColor; }
    static const QString ScrollbarFGColor;
    static QString getScrollbarFGColor() { return ScrollbarFGColor; }

    // Error note
    static const int ErrorButtonHeight;
    static int getErrorButtonHeight() { return ErrorButtonHeight; }
    static const int ErrorButtonWidth;
    static int getErrorButtonWidth() { return ErrorButtonWidth; }
    static const int ErrorButtonBottomOffset;
    static int getErrorButtonBottomOffset() { return ErrorButtonBottomOffset; }
    static const int ErrorTextMargins;
    static int getErrorTextMargins() { return ErrorTextMargins; }
    static const QString ErrorFont;
    static QString getErrorFont() { return ErrorFont; }

    // Wait animation
    static const int WaitAnimationNumFrames;
    static int getWaitAnimationNumFrames() { return WaitAnimationNumFrames; }
    static const int WaitAnimationInterval;
    static int getWaitAnimationInterval() { return WaitAnimationInterval; }

    // For use in C++ listmodels
    static QHash<int, QByteArray> roleNames();
};

#endif // CONSTANTS_HPP
