/*****************************************************************************
 * Copyright: 2012 Michael Zanetti <michael_zanetti@gmx.net>                 *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *                                                                           *
 ****************************************************************************/

// Original file: https://gitorious.org/getmewheels/getmewheels2

import QtQuick 1.1
import com.nokia.meego 1.0

Item {
    id: subTitleButton

    property alias titleText: title.text
    property alias subTitleText: subTitle.text
    property alias icon: icon.source

    signal clicked

    height: 80
    width: parent.width

    BorderImage {
        id: background
        anchors.fill: parent
        visible: mouseArea.pressed
        source: "image://theme/meegotouch-list-background-pressed-center"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: background
        onClicked: {
            subTitleButton.clicked();
        }
    }

    Label {
        id: title

        anchors {
            left: parent.left
            leftMargin: 10
            rightMargin: 10
            topMargin: 10
            top: parent.top
        }

        font.bold: true
        text: "-"
    }

    Label {
        id: subTitle
        anchors {
            top:  title.bottom
            topMargin: 10
            left: parent.left
            leftMargin: 10
            rightMargin: 10
            right: icon.left
        }
        text: ""
    }

    Image {
        id: icon

        anchors {
            right: parent.right
            rightMargin: 10
            verticalCenter: parent.verticalCenter
        }
        height: sourceSize.height
        width: sourceSize.width
        source: "image://theme/meegotouch-combobox-indicator" + (theme.inverted ? "-inverted" : "")
    }
}
