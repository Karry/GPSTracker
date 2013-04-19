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

#include <QObject>
#include <QDateTime>
#include <QSqlRecord>

#ifndef NODE_H
#define NODE_H

class Node: public QObject{
     Q_OBJECT
public:
    Node(   double latitude,
            double longitude,
            double altitude,
            double horizontalAccuracy,
            double verticalAccuracy,
            double distanceFromPrevious,
            double groundSpeed);

    Node(const Node &original);
    Node(const QSqlRecord &rec);
    Node &operator=(const Node &node);

    double latitude;
    double longitude;
    double altitude;
    double horizontalAccuracy;
    double verticalAccuracy;
    double distanceFromPrevious;
    double groundSpeed;
    QDateTime utcTimestamp;
};

#endif // NODE_H
