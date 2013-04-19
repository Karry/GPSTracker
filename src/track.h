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
#include <QDeclarativeView>
#include <QDeclarativeItem>
#include <QDateTime>

#include <QSqlRecord>

#include <QtCore/QVariant> //needed for declare metatype

// geo location
#include <QtLocation/QGeoPositionInfo>
#include <qnmeapositioninfosource.h>
#include <qgeopositioninfosource.h>
#include <qgeosatelliteinfosource.h>
#include <qgeopositioninfo.h>
#include <qgeosatelliteinfo.h>
//#include <qnetworkconfigmanager.h>
//#include <qnetworksession.h>
#include <QtSensors/QCompass>

// sql
#include <QSqlDatabase>
#include <QSqlError>

//// QtMobility API headers
#include <qmobilityglobal.h>

QTM_BEGIN_NAMESPACE
#ifndef BEARER_IN_QTNETWORK
class QNetworkSession;
#endif
class QGeoPositionInfoSource;
class QGeoPositionInfo;
class QGeoCoordinate;
class QGeoSatelliteInfoSource;
class QGeoSatelliteInfo;
QTM_END_NAMESPACE

// Use the QtMobility namespace
QTM_USE_NAMESPACE


#ifndef TRACK_H
#define TRACK_H

#include "storage.h"

// Earth radius (mean) in metres {6371, 6367}
#define EARTH_RADIUS  6371 * 1000

class Track: public QObject{
    Q_OBJECT

    Q_PROPERTY(int id
               READ getId)
    Q_PROPERTY(double length
               READ getLength)
    Q_PROPERTY(QString name
               READ getName)
    Q_PROPERTY(int nodes
               READ getNodesCount)
    Q_PROPERTY(QDateTime startTimeUtc
               READ getStartTimeUtc)
    Q_PROPERTY(QDateTime endTimeUtc
               READ getEndTimeUtc)

public:
    Track(Storage *storage, QString name, int id, bool open);
    Track(Storage *storage, QSqlRecord rec);
    virtual ~Track();
    bool close();
    double getLength();
    QDateTime getStartTimeUtc();
    QDateTime getEndTimeUtc();
    int getId();
    QString getName();
    int getNodesCount();
    QList<Node> getNodes();

private:
    void loadTrackInfo();

public
    Q_SLOTS:
    void onPositionUpdated(const QGeoPositionInfo &gpsPos);
    void onSatellitesInUseUpdated ( const QList<QGeoSatelliteInfo> & satellites );
    void onSatellitesInViewUpdated ( const QList<QGeoSatelliteInfo> & satellites );
    void onCompassChanged();

    static qreal distance(QGeoCoordinate c1, QGeoCoordinate c2);

private:
    Storage *_storage;
    int _id;
    bool _open;
    QString _name;
    QGeoCoordinate _lastCoordinate;
    double _trackLength;
    QDateTime _start;
    QDateTime _end;
    QList<Node> _nodes;
};
Q_DECLARE_METATYPE(Track*);

#endif // TRACK_H
