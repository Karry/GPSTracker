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

Page {
    id: page
    tools: commonTools

    property string title :qsTr("Settings")

    Image {
        id: pageHeader
        anchors {
            top: page.top
            left: page.left
            right: page.right
        }

        height: page.width < page.height ? 72 : 46
        width: parent.width
        source: "image://theme/meegotouch-view-header-fixed" + (theme.inverted ? "-inverted" : "")
        z: 1

        Label {
            id: header
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: 16
            }
            platformStyle: LabelStyle {
                fontFamily: "Nokia Pure Text Light"
                fontPixelSize: 32
            }
            text: page.title
        }
    }

    Flickable {
        id: pageFlickableContent
        anchors {
            top: pageHeader.bottom
            bottom: page.bottom
            left: page.left
            right: page.right
            margins: 16
        }
        contentHeight: pageFlickableContent.height
        contentWidth: pageFlickableContent.width
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: settingsColumn
            width: page.width - 2*pageFlickableContent.anchors.margins

            Item {
                anchors.left: parent.left
                anchors.right: parent.right
                height: 60
                Label {
                    text: qsTr("Night View (Black Theme)")
                    font.weight: Font.Bold
                    font.pixelSize: 26
                    anchors.verticalCenter: parent.verticalCenter
                }

                Switch {
                    anchors.right: parent.right
                    onCheckedChanged: {
                        if ((checked ? 1 : 0) != settings.nightViewMode) {
                            settings.nightViewMode = checked ? 1 : 0;
                        }
                    }
                    checked: settings.nightViewMode == 1
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
        /*
        Column {
            id: pageContent
            width: page.width - pageFlickableContent.anchors.margins * 2
            spacing: 16

            Button{
                text: qsTr("Click here!")
                onClicked: appWindow.showStatusBar = !appWindow.showStatusBar
            }
        }
        */
    }

    ScrollDecorator {
        flickableItem: pageFlickableContent
    }
}
