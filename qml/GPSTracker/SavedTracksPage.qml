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
import QtQuick 1.1
import QtMobility.feedback 1.1
import QtMultimediaKit 1.1

import com.nokia.meego 1.0

import "UIConstants.js" as UI
import gpstracker 0.1 as GpsTracker

Page {
    id: savedTracksPage
    tools: commonTools

    property string title :qsTr("Saved Tracks")
    property variant _model: ListModel {}
    property int selectedTrackId : -1;
    property string selectedTrackName : "";

    onStatusChanged: {
        if (status==PageStatus.Activating){
            core.loadTracksModel();
        }
    }
    Connections {
        target: core
        onTracksModelLoaded:{
            savedTracksPage._model = model;
            tracksView.model = savedTracksPage._model;
            console.log("SavedTracksPage.qml: model loaded");
        }
    }

    Image {
        id: pageHeader
        anchors {
            top: savedTracksPage.top
            left: savedTracksPage.left
            right: savedTracksPage.right
        }

        height: savedTracksPage.width < savedTracksPage.height ? 72 : 46
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
                fontFamily: UI.FONT_FAMILY_LIGHT //"Nokia Pure Text Light"
                fontPixelSize: UI.FONT_XLARGE
            }
            text: savedTracksPage.title
        }
    }

    ListView {
        id: tracksView
        model: savedTracksPage._model;
        anchors {
            top: pageHeader.bottom
            bottom: savedTracksPage.bottom
            left: savedTracksPage.left
            right: savedTracksPage.right
            //margins: 16
        }

        clip: true

        delegate: ListViewLine{
            id: sessionLine
            title: id+": "+name

            MouseArea{
                anchors.fill: parent
                onPressAndHold: {
                    console.log("SavedTracksPage.qml: populate menu for track "+id);
                    selectedTrackId = id;
                    selectedTrackName = name;
                    appWindow.showToolBar = false;
                    trackMenu.open();
                }
                onClicked: {
                    console.log("SavedTracksPage.qml: show track info "+id);
                    console.log("TODO ... show menu instead");

                    selectedTrackId = id;
                    selectedTrackName = name;
                    appWindow.showToolBar = false;
                    trackMenu.open();
                }
            }
        }
    }

    ScrollDecorator {
        flickableItem: tracksView //pageFlickableContent
    }

    HapticsEffect {
        id: rumbleEffect
        attackIntensity: 0.0
        attackTime: 250
        intensity: 1.0
        duration: 100
        fadeTime: 250
        fadeIntensity: 0.0
    }

    // Create a simple "Hello World"-Dialog

    Dialog {
        id: exportFileDialog

        //property variant callback;
        property string format: "gpx";

        title: Text {
            id: title
            text: qsTr("Name of exported file:")
        }

        content:Rectangle {
            id: name
            height: 120
            width: parent.width
            color: "transparent"
            //color: "blue"

            Text {
                id: queryText
                text: qsTr("Export file name:")

                color: UI.COLOR_INVERTED_FOREGROUND
                anchors.top: parent.top
            }
            Rectangle{
                anchors{
                    horizontalCenter:parent.horizontalCenter
                    margins: 20
                    bottom: parent.bottom
                }
                width: parent.width
                height: 40
                //color: "grey"
                color: UI.COLOR_BACKGROUND
                TextInput {
                    id: exportedFileName
                    font.pixelSize: 22
                    anchors{
                        centerIn: parent
                        fill: parent
                        margins: 5
                    }
                    text: "file name"
                    //color: "green"
                }
            }
        }

        buttons: Grid {
            columns: 1
            rows: 2
            spacing: 10
            Button {text: qsTr("Export"); onClicked: exportFileDialog.accept()}
            Button {text: qsTr("Cancel"); onClicked: exportFileDialog.reject()}
        }
        onStatusChanged: {
            if (status === DialogStatus.Opening){
                exportedFileName.text = selectedTrackName;
            }
        }

        onAccepted: {
            core.exportTrack(selectedTrackId, exportedFileName.text , format);
        }
    }

    Menu {
        id: trackMenu

        property int trackId;
        property variant trackItem;

        visualParent: pageStack
        MenuLayout {
            MenuItem {
                text: qsTr("Export track as GPX")
                onClicked: {
                    exportFileDialog.format = "gpx";
                    exportFileDialog.open();
                }
            }
            MenuItem {
                text: qsTr("Export track as KML")
                onClicked: {
                    exportFileDialog.format = "kml";
                    exportFileDialog.open();
                }
            }
            /*
            MenuItem {
                text: qsTr("Rename track")
                onClicked: {
                    console.log("TODO");
                }
            }
            MenuItem {
                text: qsTr("Delete track")
                onClicked: {
                    console.log("TODO");
                }
            }
            */
        }
        onStatusChanged: {
            if (status === DialogStatus.Closed) {
                appWindow.showToolBar = true;
            }
            if (status === DialogStatus.Opening) {
                if (rumbleEffect.start){
                    rumbleEffect.start();  // plays a rumble effect
                }
            }
        }
    }

}
