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

#include <QApplication>
#include <QFile>
#include <QDir>
#include <QtCore/qmath.h>
#include <qnumeric.h>
#include <QInputContext>

// sql
#include <QSqlDatabase>
#include <QSqlError>

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

// declarative
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeError>
#include <QDeclarativeItem>
#include <QtDeclarative/QDeclarativeEngine>
#include <QDeclarativeItem>

#include "gpstracker.h"
#include "tracksmodel.h"

GpsTracker::GpsTracker(QApplication *app):
    QObject(app),
    _satelitesInUse(0),
    _satelitesInView(0),
    _openedTrack(NULL),
    _tracksModel(NULL)
{

    this->_viewer = new QmlApplicationViewer();
    this->_app = app;
    this->_settings = new Settings(this);
    this->_storage = new Storage(_settings, this);
}

GpsTracker::~GpsTracker(){
    qDebug() << "GpsTracker: ending";
    if (_viewer)
        _viewer->deleteLater();
    if (_settings){
        _settings->store();
        _settings->deleteLater();
    }
    if (_positionSource){
        _positionSource->stopUpdates();
        _positionSource->deleteLater();
    }
    if (_satellitesSource){
        _satellitesSource->stopUpdates();
        _satellitesSource->deleteLater();
    }
    if (_compass){
        _compass->stop();
        _compass->deleteLater();
    }
    if (isTracking()){
        changeTracking(false);
    }
    if (_storage){
        _storage->deleteLater();
    }

    qDebug() << "GpsTracker: ended";
}

bool GpsTracker::init(){

    // load settings
    if (!this->_settings->init())
        return false;

    if (!this->_storage->init())
        return false;

    connect(_storage, SIGNAL(trackDeleted(Track*)), this, SLOT(onTrackDeleted(Track *)));

    // init QML viewer
    qRegisterMetaType<QGeoPositionInfo>("QGeoPositionInfo");
    qRegisterMetaType<QGeoCoordinate>("QGeoCoordinate");

    _viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);

    QDeclarativeContext* rootContext = _viewer->rootContext();
    rootContext->setContextProperty("core", this);
    rootContext->setContextProperty("settings", this->_settings);

    // register models to qml
    qmlRegisterUncreatableType<TracksModel> ("gpstracker", 0, 1, "TracksModel", QLatin1String("TracksModel model"));

    connect(_viewer, SIGNAL(statusChanged(QDeclarativeView::Status)),
      this, SLOT(onQmlStatusChanged(QDeclarativeView::Status)));

    _viewer->setMainQmlFile(QLatin1String("qml/GPSTracker/main.qml"));

    if (_viewer->status() == QDeclarativeView::Error)
      return false;

    _viewer->showExpanded();

    // init geo position source
    _positionSource = QGeoPositionInfoSource::createDefaultSource(this);
    // init satelites source
    _satellitesSource =  QGeoSatelliteInfoSource::createDefaultSource(this);

    if (_positionSource == 0) {
        qWarning() << "GpsTracker: no geo position source found. Use fake log input.";
        QNmeaPositionInfoSource *nmeaLocation = new QNmeaPositionInfoSource(QNmeaPositionInfoSource::SimulationMode, this);
        QFile *logFile = new QFile(QApplication::applicationDirPath()
                                   + QDir::separator() + "nmealog.txt", this);
        nmeaLocation->setDevice(logFile);
        _positionSource = nmeaLocation;
        //m_logfileInUse = true;
    }else{
        qDebug() << "GpsTracker: geo position source found";
    }

    // Listen gps position changes
    connect(_positionSource, SIGNAL(positionUpdated(QGeoPositionInfo)),
            this, SLOT(onPositionUpdated(QGeoPositionInfo)));

    // Listen to satellites source
    if (_satellitesSource != 0){
        connect(_satellitesSource, SIGNAL(satellitesInUseUpdated ( QList<QGeoSatelliteInfo> )),
                this,SLOT(onSatellitesInUseUpdated ( QList<QGeoSatelliteInfo> )));
        connect(_satellitesSource, SIGNAL(satellitesInViewUpdated ( QList<QGeoSatelliteInfo> )),
                this,SLOT(onSatellitesInViewUpdated ( QList<QGeoSatelliteInfo> )));

        //GpsTracker::onSatellitesInUseUpdated ( const QList<QGeoSatelliteInfo> & satellites ){}
        //void GpsTracker::onSatellitesInViewUpdated ( const QList<QGeoSatelliteInfo> & satellites ){}
        _satellitesSource->startUpdates();
        //_satellitesSource->requestUpdate(1000);

        qDebug() << "GpsTracker: start listening to setellistes info source";
    }

    // request geo update each second or system minimum
    // TODO: use configured value
    _positionSource->setUpdateInterval( qMax( 1 * 1000, _positionSource->minimumUpdateInterval()) );
    // Start listening GPS position updates
    //_positionSource->requestUpdate(); // request for geo position (first time)
    _positionSource->startUpdates();


    // Initialise and start the compass
    _compass = new QCompass(this);
    _compass->setDataRate ( qMin(_compass->dataRate(), 25) ); // maximum rate 25Hz

    if (! _compass->start()  || ! _compass->isActive()) {
        qWarning("GpsTracker: CompassSensor didn't start!");
        emit compassUpdate(-1);
    }else{
        connect(_compass, SIGNAL(readingChanged()),
                this, SLOT(onCompassChanged()));
    }

    qDebug() << "GpsTracker: init done";
    return true;
}


void GpsTracker::onQmlStatusChanged(QDeclarativeView::Status status){
    switch (status) {
      case QDeclarativeView::Null:
        break;
      case QDeclarativeView::Ready:
        qDebug() << "GpsTracker: QML ready...";
        onViewReady();
        break;
      case QDeclarativeView::Loading:
        qDebug() << "GpsTracker: Loading QML...";
        break;
      case QDeclarativeView::Error:
        qWarning() << "GpsTracker: Error occurs while loading qml (ui) file. Exiting";
        qWarning() << _viewer->errors();
        _app->exit(-1);
        break;
      default:
        qWarning() << "GpsTracker: udefined QML status" << status;
    }
}

void GpsTracker::onViewReady(){
    // fun began
}

void GpsTracker::onTrackDeleted(Track *track){
    if (track == _openedTrack){
        qWarning() << "GpsTracker: opened track was deleted, stop tracking";
        changeTracking(false);
    }
}

void GpsTracker::onSatellitesInUseUpdated ( const QList<QGeoSatelliteInfo> & satellites ){

    /*
    QList<QGeoSatelliteInfo>::const_iterator it ;
    qDebug()<< "GpsTracker: sattellites in use:";
    for (it = satellites.begin(); it != satellites.end(); ++it){
       QGeoSatelliteInfo satellite = *it;
       qDebug()<< "     "<< satellite;
    }
    */

    this->_satelitesInUse = satellites.size();
}

void GpsTracker::onSatellitesInViewUpdated ( const QList<QGeoSatelliteInfo> & satellites ){
    /*
    QList<QGeoSatelliteInfo>::const_iterator it ;    
    qDebug()<< "GpsTracker: sattellites in view:";
    for (it = satellites.begin(); it != satellites.end(); ++it){
       QGeoSatelliteInfo satellite = *it;
       qDebug()<< "     "<< satellite;
    }
    */

    this->_satelitesInView = satellites.size();
}

void GpsTracker::onPositionUpdated(const QGeoPositionInfo &gpsPos){
    QGeoCoordinate coordinate = gpsPos.coordinate();

    /*
    qDebug() << "GpsTracker: position changed to "
             <<  coordinate.latitude() << coordinate.longitude() ;
             */

    QVariantMap map;

    if (gpsPos.hasAttribute(QGeoPositionInfo::Direction))
            map.insert("direction", (gpsPos.attribute(QGeoPositionInfo::Direction)));

    if (gpsPos.hasAttribute(QGeoPositionInfo::GroundSpeed))
            map.insert("groundSpeed", gpsPos.attribute(QGeoPositionInfo::GroundSpeed));

    if (gpsPos.hasAttribute(QGeoPositionInfo::VerticalSpeed))
            map.insert("verticalSpeed", gpsPos.attribute(QGeoPositionInfo::VerticalSpeed));

    if (gpsPos.hasAttribute(QGeoPositionInfo::MagneticVariation))
            map.insert("magneticVariation", gpsPos.attribute(QGeoPositionInfo::MagneticVariation));

    if (gpsPos.hasAttribute(QGeoPositionInfo::HorizontalAccuracy))
            map.insert("horizontalAccuracy", gpsPos.attribute(QGeoPositionInfo::HorizontalAccuracy));

    if (gpsPos.hasAttribute(QGeoPositionInfo::VerticalAccuracy))
            map.insert("verticalAccuracy", gpsPos.attribute(QGeoPositionInfo::VerticalAccuracy));

    emit positionUpdated(gpsPos.timestamp(),
                         coordinate.latitude(), coordinate.longitude(), coordinate.altitude(),
                         _satelitesInView, _satelitesInUse,
                          QVariant::fromValue( map ) );
}

void GpsTracker::onCompassChanged(){
    QCompassReading *data = _compass->reading();
    //qDebug() << "GpsTracker: compass"<< data->azimuth() << "(" << qFloor( data->calibrationLevel()*100) << "%)";
    emit compassUpdate(data->azimuth());
}

bool GpsTracker::isTracking(){
    return _openedTrack != NULL;
}

void GpsTracker::changeTracking(bool tracking){
    if (tracking){
        if (!isTracking()){
            _openedTrack = _storage->createNewTrack();

            if (_openedTrack == NULL){
                qWarning()<< "GpsTracker: creating track failed";
                return;
            }
            // connect newly created track to signals
            connect(_positionSource, SIGNAL(positionUpdated(QGeoPositionInfo)),
                    _openedTrack, SLOT(onPositionUpdated(QGeoPositionInfo)));

            if (_satellitesSource != 0){
                connect(_satellitesSource, SIGNAL(satellitesInUseUpdated ( QList<QGeoSatelliteInfo> )),
                        _openedTrack,SLOT(onSatellitesInUseUpdated ( QList<QGeoSatelliteInfo> )));
                connect(_satellitesSource, SIGNAL(satellitesInViewUpdated ( QList<QGeoSatelliteInfo> )),
                        _openedTrack,SLOT(onSatellitesInViewUpdated ( QList<QGeoSatelliteInfo> )));
            }

            if ( _compass->isActive()) {
                connect(_compass, SIGNAL(readingChanged()),
                        _openedTrack, SLOT(onCompassChanged()));
            }

            QDeclarativeContext* rootContext = _viewer->rootContext();
            rootContext->setContextProperty("track", _openedTrack);
            emit trackingChanged(true);
        }
    }else{
        if (isTracking()){
            disconnect(_positionSource, SIGNAL(positionUpdated(QGeoPositionInfo)),
                    _openedTrack, SLOT(onPositionUpdated(QGeoPositionInfo)));

            if (_satellitesSource != 0){
                disconnect(_satellitesSource, SIGNAL(satellitesInUseUpdated ( QList<QGeoSatelliteInfo> )),
                        _openedTrack,SLOT(onSatellitesInUseUpdated ( QList<QGeoSatelliteInfo> )));
                disconnect(_satellitesSource, SIGNAL(satellitesInViewUpdated ( QList<QGeoSatelliteInfo> )),
                        _openedTrack,SLOT(onSatellitesInViewUpdated ( QList<QGeoSatelliteInfo> )));
            }

            if ( _compass->isActive()) {
                disconnect(_compass, SIGNAL(readingChanged()),
                        _openedTrack, SLOT(onCompassChanged()));
            }

            if (_openedTrack->close()){
                _openedTrack = NULL;
                QDeclarativeContext* rootContext = _viewer->rootContext();
                rootContext->setContextProperty("track", _openedTrack);
                emit trackingChanged(false);
            }
        }
    }
}

void GpsTracker::loadTracksModel(){
    if (_tracksModel==NULL){
        _tracksModel = new TracksModel(_storage);
        qDebug() << "GpsTracker: load tracks model (" << _tracksModel->rowCount() << ")";
        emit tracksModelLoaded(_tracksModel); // todo write a model
    }
}

void GpsTracker::exportTrack(int trackId, QString fileName, QString format){
    qDebug() << "GpsTracker: export track (" << trackId << "," << fileName << "," << format << ")";

    if (format != "kml" && format != "gpx"){
        qWarning() << "GpsTracker: unsuported export format" << format;
        return;
    }

    Track *track = _storage->getTrack(trackId);
    if (track == NULL){
        qWarning() << "GpsTracker: Track id" << trackId << "doesn't exists";
        return;
    }

    QFile file(fileName+"."+format);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);

        // TODO: write to file in different thread...
        if (format == "gpx")
            this->exportTrackToGPX(&out, track);
        if (format == "kml")
            this->exportTrackToKML(&out, track);


        file.close();
    }else{
        qWarning() << "GpsTracker: opening file" << file.fileName() << "for writing failed!";
    }
}

void GpsTracker::renameTrack(int trackId, QString newName){
    qDebug() << "GpsTracker: export track (" << trackId << "," << newName << ")";

    Track *track = _storage->getTrack(trackId);
    if (track == NULL){
        qWarning() << "GpsTracker: Track id" << trackId << "doesn't exists";
        return;
    }

    track->rename(newName);
}

void GpsTracker::deleteTrack(int trackId){
    qDebug() << "GpsTracker: delete track (" << trackId << ")";

    Track *track = _storage->getTrack(trackId);
    if (track == NULL){
        qWarning() << "GpsTracker: Track id" << trackId << "doesn't exists";
        return;
    }

    track->deleteTrack();
}

void GpsTracker::exportTrackToGPX(QTextStream *out, Track *track){

    *out << "<?xml version='1.0' encoding='UTF-8'?>\n";
    *out << "<gpx version='1.1'\n";
    *out << "creator='GPSTracker - https://github.com/Karry'\n";
    *out << "xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'\n";
    *out << "xmlns='http://www.topografix.com/GPX/1/1'\n";
    *out << "xsi:schemaLocation='http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd'>\n";

    *out <<  "<trk>\n<name>" + track->getName() + "</name>\n<trkseg>\n";

    QList<Node> nodes = track->getNodes();
    QList<Node>::iterator it = nodes.begin();
    while (it != nodes.end()){
        Node node = ((Node)(*it));

        *out << "<trkpt lat='" << node.latitude << "' lon='" << node.longitude << "'>\n";
        *out << "\t<time>" << node.utcTimestamp.toString("yyyy-MM-ddThh:mm:ssZ") << "</time>\n"; // 2010-03-23T11:01:50Z
        if (node.altitude != 0)
            *out << "\t<ele>" << node.altitude << "</ele>\n";
        if (node.groundSpeed >=0)
            *out << "\t<speed>" << node.groundSpeed << "</speed>\n";
        if (node.horizontalAccuracy>0)
            *out << "\t<hdop>" << node.horizontalAccuracy << "</hdop>\n";
        if (node.verticalAccuracy>0)
            *out << "\t<vdop>" << node.verticalAccuracy << "</vdop>\n";
        *out << "</trkpt>\n";

        it++;
    }

    *out <<  "</trkseg>\n</trk>\n";
    *out <<  "</gpx>\n";
}

void GpsTracker::exportTrackToKML(QTextStream *out, Track *track){

    *out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    *out << "<kml xmlns=\"http://www.opengis.net/kml/2.2\"\n";
    *out << " xmlns:gx=\"http://www.google.com/kml/ext/2.2\"\n";
    *out << " xmlns:kml=\"http://www.opengis.net/kml/2.2\" \n";
    *out << " xmlns:atom=\"http://www.w3.org/2005/Atom\">\n";
    *out << "<Document><name>" + track->getName() + "</name><open>1</open><Style id=\"path0Style\"><LineStyle><color>ffff4040</color><width>6</width></LineStyle></Style>\n";
    *out << "  <StyleMap id=\"waypoint\"><IconStyle><scale>1.2</scale><Icon><href>http://maps.google.com/mapfiles/kml/pal4/icon61.png</href></Icon></IconStyle></StyleMap>\n";

    double lastAlt = qQNaN();
    QList<Node> nodes = track->getNodes();
    QList<Node>::iterator it = nodes.begin();
    while (it != nodes.end()){
        Node node = ((Node)(*it));

        *out << "" << node.longitude << "," << node.latitude << ",";
        if (node.altitude == qQNaN()){
            *out << lastAlt;
        }else{
            *out << node.altitude;
            *out << " ";
            lastAlt = node.altitude;
        }
        *out << "\n";

        it++;
    }

    *out << "</coordinates></LineString></MultiGeometry></Placemark></Folder></Document></kml>\n";

}
