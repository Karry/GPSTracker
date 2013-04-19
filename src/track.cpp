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

#include <QDateTime>

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

#include <qnumeric.h>
#include "track.h"
#include "node.h"

Track::Track(Storage *storage, QString name, int id, bool open):
    _storage(storage),
    _id(id),
    _open(open),
    _name(name),
    _trackLength(0),
    _start(QDateTime::currentDateTimeUtc()),
    _end(QDateTime::currentDateTimeUtc()),
    _nodes(QList<Node>())
{
}

Track::Track(Storage *storage, QSqlRecord rec):
    _storage(storage),
    _trackLength(-1),
    _nodes(QList<Node>())
{
    // TODO: check values?
    _id = rec.value("id").toInt();
    _name = rec.value("name").toString();
    _open = rec.value("open").toBool();
}

Track::~Track(){
}

bool Track::close(){
    if (_open){
        _end = QDateTime::currentDateTimeUtc();
        return _storage->closeTrack(_id);
    }
    return false;
}

void Track::onSatellitesInUseUpdated ( const QList<QGeoSatelliteInfo> & satellites ){
    Q_UNUSED(satellites);
}

void Track::onSatellitesInViewUpdated ( const QList<QGeoSatelliteInfo> & satellites ){
    Q_UNUSED(satellites);
}

void Track::onPositionUpdated(const QGeoPositionInfo &gpsPos){
    if (!_open)
        return;

    QGeoCoordinate coordinate = gpsPos.coordinate();

    double groundSpeed = (gpsPos.hasAttribute(QGeoPositionInfo::GroundSpeed))
            ? gpsPos.attribute(QGeoPositionInfo::GroundSpeed)
              :qQNaN();

    double horizontalAccuracy = (gpsPos.hasAttribute(QGeoPositionInfo::HorizontalAccuracy))
            ? gpsPos.attribute(QGeoPositionInfo::HorizontalAccuracy)
            :qQNaN();

    double verticalAccuracy = (gpsPos.hasAttribute(QGeoPositionInfo::VerticalAccuracy))
            ? gpsPos.attribute(QGeoPositionInfo::HorizontalAccuracy)
            :qQNaN();

    double distance = Track::distance(_lastCoordinate, coordinate);
    if (horizontalAccuracy > _storage->getSettings()->getMaximumAccuracy()) // don't compute distance when accuracy is too high
        distance = 0;
    _trackLength += distance;

    //_end = QDateTime::currentDateTimeUtc();
    Node node(coordinate.latitude(), coordinate.longitude(), coordinate.altitude(),
                horizontalAccuracy, verticalAccuracy,
                distance,
                groundSpeed);
    if (_storage->addNode(_id,node)){
        _nodes.append(node);
        _lastCoordinate = coordinate;
        _end = node.utcTimestamp;
    }

}

void Track::loadTrackInfo(){
    // load track info from storage
    _nodes = _storage->loadNodes(this->_id);
    qDebug() << "Track: loaded" << _nodes.size() << "nodes for track id" << _id;
    if (_nodes.size() > 0){
        _start = _nodes.first().utcTimestamp;
        _end = ((Node)_nodes.last()).utcTimestamp;

        QList<Node>::iterator it = _nodes.begin();
        while (it != _nodes.end()){
            _trackLength += ((Node)(*it)).distanceFromPrevious;
            it++;
        }
    }
}

double Track::getLength(){
    if (_trackLength<0)
        loadTrackInfo();
    return _trackLength;
}

QDateTime Track::getStartTimeUtc(){
    if (!_start.isValid())
        loadTrackInfo();
    return _start;
}

QDateTime Track::getEndTimeUtc(){
    if (!_end.isValid())
        loadTrackInfo();
    return _end;
}

QString Track::getName(){
    return _name;
}

int Track::getId(){
    return _id;
}

int Track::getNodesCount(){
    if (_trackLength<0)
        loadTrackInfo();
    return _nodes.size();
}

QList<Node> Track::getNodes(){
    if (_trackLength<0)
        loadTrackInfo();
    return _nodes;
}

void Track::onCompassChanged(){
}

qreal Track::distance(QGeoCoordinate c1, QGeoCoordinate c2){
    if (!c1.isValid() || !c2.isValid())
        return 0;

    return c1.distanceTo(c2);
}
