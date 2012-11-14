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
#include "Constants.hpp"

// Constants

// General stuff
const QString Constants::SpotifyGreen = QLatin1String("#7AB800");
const QString Constants::DimmedTextColor = QLatin1String("#979797");
const QString Constants::StandardFont = QLatin1String("Nokia Sans");
const int Constants::NaviButtonWidth = 85;
const int Constants::StandardMargin = 20;
const QString Constants::Version = QLatin1String("1.1");

// Playing screen
const int Constants::ControlsHeight = 130;
const int Constants::ControlWidth = 193;
const int Constants::PlayIndicatorHeight = 10;
const int Constants::AlbumArtWidth = 300;
const int Constants::ShuffleButtonWidth = 108;
const int Constants::ShuffleButtonHeight = 41;

// Lists
const int Constants::ListElementHeight = 160;
const int Constants::ListSeparatorHeight = 2;
const int Constants::ListIconWidth = 148;
const int Constants::ListIconHeight = 148;
const int Constants::ListIconMargin = 6;
const int Constants::ListEndMargin = 10;
const int Constants::ListMainTextSize = 49;
const int Constants::ListSecondaryTextSize = 38;
const int Constants::SublistIconHeight = 50;
const int Constants::SublistIconWidth = 50;
const int Constants::RadioButtonHeight = 148;
const int Constants::RadioButtonWidth = 148;
const int Constants::ToggleButtonHeight = 41;
const int Constants::ToggleButtonWidth = 67;
const int Constants::ListScrollBy = 3;

// Scrollbar appearence
const int Constants::ScrollbarMargin = 4;
const int Constants::ScrollbarWidth = 15;
const int Constants::ScrollbarRadius = 6;
const QString Constants::ScrollbarBGColor = QLatin1String("#2B2C2D");
const QString Constants::ScrollbarFGColor = QLatin1String("#979797");


// Error note
const int Constants::ErrorButtonHeight = 100;
const int Constants::ErrorButtonWidth = 430;
const int Constants::ErrorButtonBottomOffset = 140;
const int Constants::ErrorTextMargins = 5;
const QString Constants::ErrorFont = QLatin1String("Nokia Pure");

// Wait animation
const int Constants::WaitAnimationNumFrames = 7;
const int Constants::WaitAnimationInterval = 200;

QHash<int, QByteArray> Constants::roleNames()
{
    QHash<int, QByteArray> names;

    names[TextRole] = "text";
    names[IdentifierRole] = "identifier";
    names[SecondRowTextRole] = "second_row_text";
    names[SecondRowTextColorRole] = "second_row_text_color";
    names[IconRole] = "icon";
    names[SublistRole] = "sublist";
    names[ToggleItemRole] = "toggle_item";
    names[ToggleStateRole] = "toggle_state";
    names[BackgroundRole] = "background";
    names[DimmedRole] = "dimmed";

    return names;
}
