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
import QtMobility.sensors 1.1

Page {
    id: page
    tools: commonTools

    property string title : "GPS Tracker"   

    function refreshTrackInfo(){
        if (  (typeof track)!="undefined" && track!=null ){
            trackNameInfo.value = track.name;
            trackLengthInfo.value = track.length+" m";
            trackNodesInfo.value = track.nodes;
        }
    }

    Connections {
        target: core

        onPositionUpdated: {
            /*
            console.debug("MainPage.qml: positionUpdated "
                          +timestamp+", "
                          +latitude+", "
                          +longitude+", "
                          +altitude
                          );

            for (var prop in attributes) {
                 console.log("    ", prop, "=", attributes[prop])
            }
            */

            // TODO: format values by config and use predefined units (metrics or imperial)
            positionInfo.value = latitude + " " + longitude +
                    (attributes.horizontalAccuracy!==undefined? " (±" + attributes.horizontalAccuracy + " m)":"");
            altitudeInfo.value = (altitude == undefined || (altitude+"") == "NaN")
                    ? "?"
                    : ( settings.formatSmallDistance( altitude ) +
                        (attributes.verticalAccuracy !==undefined? " (±" + settings.formatSmallDistance(attributes.verticalAccuracy, false, false) + ")":""));
            satelitesInfo.value = countSatellitesInView + "/" + countSatellitesInUse;
            speedInfo.value = attributes.groundSpeed === undefined? "?" : attributes.groundSpeed;
            lastUpdateInfo.value = timestamp;

            refreshTrackInfo();
        }
        onCompassUpdate:{
            //console.debug("MainPage.qml: compass azimut: "+azimut);
            if (azimut<0){
                compass.rotation = 0;
                compass.source= "img/compass_inactive.png";
            }else{
                var invert = true;
                var screenRotation = 0;
                if (screen!==undefined){
                    //Screen.Portrait - when the width of the screen is less than or equal to the height.
                    if (screen.currentOrientation === Screen.Portrait){
                        invert = true;
                        screenRotation = 0;
                    }
                    //Screen.Landscape - when the width of the screen is greater than the height.
                    if (screen.currentOrientation === Screen.Landscape){
                        invert = true;
                        screenRotation = -90;
                    }
                    // Screen.PortraitInverted - when the width of the screen is less than or equal to the height, and the screen is considered to be upside-down.
                    if (screen.currentOrientation === Screen.PortraitInverted){
                        invert = false;
                        screenRotation = 0;
                    }
                    // Screen.LandscapeInverted
                    if (screen.currentOrientation === Screen.LandscapeInverted){
                        invert = false;
                        screenRotation = +90;
                    }
                }

                compass.rotation = (azimut*(invert?-1:1))+screenRotation;
                compass.source= "img/compass.png";
            }
        }
        onTrackingChanged:{
            trackInfoWrapper.visible = tracking;
            trackingSwitch.checked = tracking;
            refreshTrackInfo();
        }
    } 

    Flickable {
        id: pageFlickableContent
        anchors {
            top: page.top
            bottom: page.bottom
            left: page.left
            right: page.right
            //margins: 16
        }
        contentHeight: pageContent.height
        contentWidth: pageContent.width
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: pageTitle
            width: page.width

            Image {
                id: pageHeader

                /* QML Column: Cannot specify top, bottom, verticalCenter, fill or centerIn anchors for items inside Column */
                /*
                anchors {
                    top: pageTitle.top
                    left: pageTitle.left
                    right: pageTitle.right
                }
                */


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
        }


        Column {
            id: pageContent
            anchors.top: pageTitle.bottom;
            width: page.width - pageFlickableContent.anchors.margins * 2
            spacing: 16
            anchors.leftMargin:  spacing
            anchors.rightMargin: spacing

            Item {
                id: trackingSwitchWrapper
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2*parent.spacing

                height: 60
                Label {
                    text: qsTr("Save track")
                    //font.weight: Font.Bold
                    font.pixelSize: 26
                    anchors.verticalCenter: parent.verticalCenter
                }

                Switch {
                    id: trackingSwitch
                    anchors.right: parent.right
                    onCheckedChanged: {
                        if ((checked ? 1 : 0) != core.tracking) {
                            core.tracking = checked ? 1 : 0;
                        }
                    }
                    checked: core.tracking == 1
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            InfoGroup {
                id: infoWrapper
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2*parent.spacing
                height: 5.5 * 60

                InfoItem{
                    id: positionInfo
                    height: 60
                    y: (height * .25)
                    //anchors.top: parent.top;
                    label: qsTr("POSITION")
                    value: "XX.XX N XX.XX W"
                }
                InfoItem{
                    id: altitudeInfo
                    anchors.top: positionInfo.bottom
                    height: 60
                    label: qsTr("ALTITUDE")
                    value: ""
                }
                InfoItem{
                    id: satelitesInfo
                    anchors.top: altitudeInfo.bottom
                    height: 60
                    label: qsTr("SATELITES IN VIEW / USE")
                    value: ""
                }
                InfoItem{
                    id: speedInfo
                    anchors.top: satelitesInfo.bottom
                    height: 60
                    label: qsTr("SPEED")
                    value: ""
                }
                InfoItem{
                    id: lastUpdateInfo
                    anchors.top: speedInfo.bottom
                    height: 60
                    label: qsTr("LAST UPDATE")
                    value: ""
                }

                Image {
                    id: compass
                    source: "img/compass_inactive.png"
                    anchors.top: positionInfo.bottom
                    anchors.bottom: lastUpdateInfo.top
                    anchors.right: infoWrapper.right
                    width: height
                }
            }

            InfoGroup {
                id: trackInfoWrapper
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2*parent.spacing
                height: 5.5 * 60
                visible: core.tracking

                InfoItem{
                    id: trackNameInfo
                    height: 60
                    y: (height * .25)
                    //anchors.top: parent.top;
                    label: qsTr("TRACK NAME")
                }
                InfoItem{
                    id: trackLengthInfo
                    height: 60
                    anchors.top: trackNameInfo.bottom
                    anchors.left: parent.left
                    label: qsTr("LENGTH")
                }
                InfoItem{
                    id: trackNodesInfo
                    height: 60
                    anchors.top: trackNameInfo.bottom
                    anchors.left: parent.horizontalCenter
                    label: qsTr("NODES")
                }


            }

        } // pageContent

    }

    ScrollDecorator {
        flickableItem: pageFlickableContent
    }
}
