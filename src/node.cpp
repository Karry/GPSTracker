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

#include <QVariant>
#include <QDebug>

#include "node.h"

Node::Node( double latitude,
            double longitude,
            double altitude,
            double horizontalAccuracy,
            double verticalAccuracy,
            double distanceFromPrevious,
            double groundSpeed):

    latitude(latitude),
    longitude(longitude),
    altitude(altitude),
    horizontalAccuracy(horizontalAccuracy),
    verticalAccuracy(verticalAccuracy),
    distanceFromPrevious(distanceFromPrevious),
    groundSpeed(groundSpeed),
    utcTimestamp(QDateTime::currentDateTimeUtc())
{

}

Node::Node(const Node &original):
    QObject(NULL),
    latitude(original.latitude),
    longitude(original.longitude),
    altitude(original.altitude),
    horizontalAccuracy(original.horizontalAccuracy),
    verticalAccuracy(original.verticalAccuracy),
    distanceFromPrevious(original.distanceFromPrevious),
    groundSpeed(original.groundSpeed),
    utcTimestamp(original.utcTimestamp)
{

}

Node::Node(const QSqlRecord &rec){
    //("(track_id,  timestamp,  latitude,  longitude,  altitude,  horiz_accuracy,  vert_accuracy,  distance_from_prev,  ground_speed) ");
    // TODO: check values?

    latitude = rec.value("latitude").toDouble();
    longitude = rec.value("longitude").toDouble();
    altitude  = rec.value("altitude").toDouble();
    horizontalAccuracy = rec.value("horiz_accuracy").toDouble();
    verticalAccuracy = rec.value("vert_accuracy").toDouble();
    distanceFromPrevious = rec.value("distance_from_prev").toDouble();
    groundSpeed = rec.value("ground_speed").toDouble();

    utcTimestamp = rec.value("timestamp").toDateTime();
}

Node &Node::operator=(const Node &node){

    latitude = node.latitude;
    longitude = node.longitude;
    altitude = node.altitude;
    horizontalAccuracy = node.horizontalAccuracy;
    verticalAccuracy = node.verticalAccuracy;
    distanceFromPrevious = node.distanceFromPrevious;
    groundSpeed = node.groundSpeed;
    utcTimestamp = node.utcTimestamp;

    return *this;
}
