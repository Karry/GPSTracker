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

PageStackWindow {
    id: appWindow

    initialPage: mainPage

    Component.onCompleted: {
        theme.inverted = settings.nightViewMode;
    }

    Connections {
        target: settings

        onNightViewModeChanged: {
            console.debug("main.qml: nightViewModeChanged");
            theme.inverted = settings.nightViewMode;
        }
    }

    MainPage {
        id: mainPage
    }
    SavedTracksPage{
        id: savedTracksPage
    }
    SettingsPage{
        id: settingsPage
    }

    ToolBarLayout {
        id: commonTools

        ToolIcon {
            visible: pageStack.depth > 1
            platformIconId: "toolbar-back"
            onClicked: {
                pageStack.pop();
            }
        }

        ToolIcon {
            visible: pageStack.depth == 1
            platformIconId: "toolbar-directory"
            anchors.right: menuToolIcon.left
            anchors.rightMargin: 16
            onClicked: {
                pageStack.push(savedTracksPage)
            }
        }
        ToolIcon {
            id: menuToolIcon
            platformIconId: "toolbar-view-menu"
            onClicked: {
                appWindow.showToolBar = false;
                myMenu.open();
            }
        }
    }

    QueryDialog {
        id: aboutDialog
        titleText: "GPSTracker"
        message: "(C) 2012 Karry\n0.1"
    }

    Menu {
        id: myMenu
        visualParent: pageStack
        MenuLayout {
            MenuItem {
                text: qsTr("About")
                onClicked: {
                    aboutDialog.open();
                }
            }
            MenuItem {
                text: qsTr("Settings")
                onClicked: {
                    pageStack.push(settingsPage)
                }
            }
        }
        onStatusChanged: {
            if (status === DialogStatus.Closed) {
                appWindow.showToolBar = true;
            }
        }
    }
}
