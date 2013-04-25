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

#include <QtCore/QObject>
//#include <QtLocation/QGeoPositionInfo>
#include <QDeclarativeView>
#include <QDeclarativeItem>
#include <QtSensors/QCompass>

// sql
#include <QSqlDatabase>
#include <QSqlError>

//// QtMobility API headers
#include <qmobilityglobal.h>

#include "qmlapplicationviewer.h"
#include "settings.h"
#include "track.h"
#include "storage.h"
#include "tracksmodel.h"

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



#ifndef GPSTRACKER_H
#define GPSTRACKER_H

class GpsTracker: public QObject{
    Q_OBJECT
    Q_DISABLE_COPY(GpsTracker)

    Q_PROPERTY(bool tracking
               READ isTracking
               WRITE changeTracking
               NOTIFY trackingChanged)
public:
    GpsTracker(QApplication *app);
    ~GpsTracker();
    bool init();
    bool isTracking();
    void changeTracking(bool tracking);
    Q_INVOKABLE void loadTracksModel();
    Q_INVOKABLE void exportTrack(int trackId, QString fileName, QString format);
    Q_INVOKABLE void renameTrack(int trackId, QString newName);
    Q_INVOKABLE void deleteTrack(int trackId);

Q_SIGNALS:
    void positionUpdated(QDateTime timestamp,
                         const double latitude, const double longitude, const double altitude,
                         const int countSatellitesInView, const int countSatellitesInUse,
                         QVariant attributes);
    void compassUpdate(const qreal azimut);
    void trackingChanged(bool tracking);
    void tracksModelLoaded(TracksModel *model);

public
    Q_SLOTS:
    void onPositionUpdated(const QGeoPositionInfo &gpsPos);
    void onSatellitesInUseUpdated ( const QList<QGeoSatelliteInfo> & satellites );
    void onSatellitesInViewUpdated ( const QList<QGeoSatelliteInfo> & satellites );
    void onCompassChanged();
    void onTrackDeleted(Track *track);

private:
    void onViewReady();
    void exportTrackToGPX(QTextStream *out, Track *track);
    void exportTrackToKML(QTextStream *out, Track *track);

private
    Q_SLOTS:
    void onQmlStatusChanged(QDeclarativeView::Status status);

private:
    Settings *_settings;
    QmlApplicationViewer *_viewer;
    QApplication *_app;
    QGeoPositionInfoSource *_positionSource;
    QGeoSatelliteInfoSource *_satellitesSource;
    QCompass *_compass;
    int _satelitesInUse;
    int _satelitesInView;
    Track *_openedTrack;
    Storage *_storage;
    TracksModel *_tracksModel;
};

#endif // GPSTRACKER_H
