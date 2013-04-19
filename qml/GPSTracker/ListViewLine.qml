/*
 *   Copyright (C) 2013 Lukáš Karas <lukas.karas@centrum.cz>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

import QtQuick 1.1
import com.nokia.meego 1.0
import "UIConstants.js" as UI // /usr/lib/qt4/imports/com/meego/UIConstants.js

Rectangle {

    property string title: "title"

    id: myDelegate
    height: titleText.height * 2 + 5
    width: parent.width
    color: "transparent"

    Label {
        id: titleText
        text: myDelegate.title
        platformStyle: LabelStyle {
            fontFamily:  UI.FONT_FAMILY // "Nokia Pure Text"
            fontPixelSize: UI.FONT_LARGE
        }
        anchors{
            leftMargin: 16
            rightMargin: 16
            topMargin: 2
            top: parent.top
            right: parent.right
            left: parent.left
        }
    }

    Image {
        id: separator
        anchors{
            //margins: 16
            leftMargin: 16
            rightMargin: 16
            bottom: parent.bottom
            right: parent.right
            left: parent.left
        }

        source: "image://theme/meegotouch-separator-background-horizontal"+ (theme.inverted ? "-inverted" : "")
    }
}
