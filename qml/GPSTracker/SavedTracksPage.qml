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

    Dialog{
        id: confirmDialog

        content:Rectangle {
            id: confirmDialogContent
            height: 120
            width: parent.width
            color: "transparent"

            Text {
                id: confirmDialogTitle
                text: qsTr("Realy?")
                color: UI.COLOR_INVERTED_FOREGROUND
                anchors.top: parent.top
                wrapMode: Text.WordWrap
                font.pixelSize: 28
            }
        }
        buttons: Grid {
            columns: 1
            rows: 2
            spacing: 10
            Button {id: confirmBtn; text: qsTr("OK"); onClicked: confirmDialog.accept()}
            Button {id: rejectBtn;  text: qsTr("Cancel"); onClicked: confirmDialog.reject()}
        }
        onAccepted: {
            console.log("delete track " + selectedTrackId);
            core.deleteTrack(selectedTrackId);
        }
    }

    Dialog {
        id: exportFileDialog

        property string format: "gpx";

        content:Rectangle {
            id: name
            height: 120
            width: parent.width
            color: "transparent"

            Text {
                id: queryText
                text: qsTr("Export file name:")
                font.pixelSize: 22
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
                    onActiveFocusChanged: {
                       if (!exportedFileName.activeFocus) exportedFileName.closeSoftwareInputPanel();
                    }
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
                exportedFileName.forceActiveFocus();
            }
            if (status === DialogStatus.Closing){
                exportedFileName.focus = false;
            }
        }

        onAccepted: {
            core.exportTrack(selectedTrackId, exportedFileName.text , format);
        }
    }
    Dialog {
        id: renameDialog

        property string format: "gpx";

        content:Rectangle {
            id: renameDialogContent
            height: 120
            width: parent.width
            color: "transparent"

            Text {
                text: qsTr("New track name:")
                font.pixelSize: 22
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
                    id: trackName
                    font.pixelSize: 22
                    anchors{
                        centerIn: parent
                        fill: parent
                        margins: 5
                    }
                    text: "file name"

                    onAccepted: renameDialog.accept();
                    onActiveFocusChanged: {
                       if (!trackName.activeFocus) trackName.closeSoftwareInputPanel();
                    }
                }
            }
        }

        buttons: Grid {
            columns: 1
            rows: 2
            spacing: 10
            Button {text: qsTr("Rename"); onClicked: renameDialog.accept()}
            Button {text: qsTr("Cancel"); onClicked: renameDialog.reject()}
        }
        onStatusChanged: {
            if (status === DialogStatus.Opening){
                trackName.text = selectedTrackName;

                trackName.forceActiveFocus();
            }
            if (status === DialogStatus.Closing){
                trackName.focus = false;
            }
        }

        onAccepted: {
            core.renameTrack(selectedTrackId, trackName.text);
        }
    }

    ContextMenu {
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
            MenuItem {
                text: qsTr("Rename track")
                onClicked: {
                    renameDialog.open();
                }
            }
            MenuItem {
                text: qsTr("Delete track")
                onClicked: {                    
                    confirmDialogTitle.text = qsTr("Do you want delete track \n%1 ?").arg(selectedTrackName);
                    console.log(confirmDialogTitle.text);
                    confirmDialog.open();
                }
            }
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
