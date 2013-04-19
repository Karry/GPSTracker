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

Item {
    property string label: "label"
    property string value: "value"

    anchors.horizontalCenter: parent.horizontalCenter
    width: parent.width

    Label {
        text: label
        //font.weight: Font.Bold
        opacity: .5
        font.pixelSize: 16
        anchors.top: parent.top
        x:10
    }
    Label {
        text: value
        font.weight: Font.Bold
        font.pixelSize: 26
        x: 20
        anchors.verticalCenter: parent.verticalCenter
    }

}
