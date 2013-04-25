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

#include <QDebug>
#include <QDir>
#include <QDateTime>

#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlDriver>
#include <qnumeric.h>

#include "storage.h"
#include "track.h"
#include "settings.h"

Storage::Storage(Settings *settings, QObject * parent):
    QObject(parent),
  _settings(settings)
{
}

Storage::~Storage(){
    _db.close();
    QSqlDatabase::removeDatabase("storage");

    // TODO: destroy all tracks
    QList<Track *>::iterator it;
    for (it = _allTracks.begin(); it != _allTracks.end(); ++it){
        (*it)->deleteLater();
    }
}

Track * Storage::createNewTrack(){
    int trackId = ++_lastId;
    QDateTime now = QDateTime::currentDateTime();
    QString trackName = now.toString("dd.MM.yyyy hh:mm:ss"); // TODO: load format from configuration

    QSqlQuery sql(_db);
    sql.prepare("INSERT INTO tracks (id, name, open, creation_time) VALUES (:id, :name, 1, :now)");
    sql.bindValue(":id", trackId);
    sql.bindValue(":name", QVariant(trackName));
    sql.bindValue(":now", QDateTime::currentDateTimeUtc());
    sql.exec();

    qDebug()<< "Storage: " << sql.executedQuery() << sql.boundValues();

    if (sql.lastError().isValid()){
        qWarning() << "Storage: creating track failed" << sql.lastError();
        return NULL;
    }

    Track *track = new Track(this, trackName, trackId, true);
    _allTracks.prepend(track);
    emit trackCreated(track);
    return track;
}

bool Storage::init(){
    if (!this->initDatabase())
        return false;

    if (!createTables())
       return false;

    loadInfo();

    return true;
}

Settings *Storage::getSettings(){
    return _settings;
}

QList<Track*> Storage::getAllTracks(){
    return _allTracks;
}

Track* Storage::getTrack(int trackId){

    QList<Track*>::Iterator it = _allTracks.begin();
    while (it != _allTracks.end()){
        Track *t = *it;
        if (t->getId()==trackId){
            return t;
        }
        it++;
    }
    return NULL;
}

void Storage::loadInfo(){
    // load lastId
    _lastId = 0;
    QString sql("SELECT MAX(`id`) AS lastId FROM `tracks`;");

    QSqlQuery q = _db.exec(sql);
    if (!q.lastError().isValid()){
        if (q.next()){
            QVariant val = q.value(0);
            if (!val.isNull()){
                bool *ok = new bool[1];
                _lastId = val.toInt(ok);
                if (! *ok)
                    qWarning() << "Storage: loading lastId value failed"<<val;
                qDebug()<<"Storage: last id: "<<_lastId;
                delete ok;
            }
        }
    }else{
        qWarning() << "Storage: failed to get last track id " << q.lastError();
    }

    // load basic info about all tracks
    // TODO: load _allTracks lazy
    q = _db.exec("SELECT * FROM `tracks` ORDER BY id DESC;");
    if (!q.lastError().isValid()){
        while (q.next()){
            //QSqlRecord rec = q.driver()->record( q );
            Track *t = new Track(this, q.record());
            _allTracks.append(t);
        }
    }else{
        qWarning() << "Storage: failed to load basic tracks info " << q.lastError();
    }

}

bool Storage::initDatabase(){
    // Find QSLite driver
    this->_db = QSqlDatabase::addDatabase("QSQLITE", "storage");
    if (!this->_db.isValid()){
        qWarning() << "GpsTracker: could not find QSQLITE backend";
        return false;
    }

    #ifdef Q_OS_LINUX
    // NOTE: We have to store database file into user home folder in Linux
    QString path(QDir::home().path());
    path.append(QDir::separator()).append(".gpstracker");
    path = QDir::toNativeSeparators(path);

    if (!QDir::home().mkpath(path)){
        qWarning() << "Settings: Failed to cretate directory"<< path;
        return false;
    }

    path.append(QDir::separator()).append("storage.sqlite");
    path = QDir::toNativeSeparators(path);
    _db.setDatabaseName(path);
    #else
    // NOTE: File exists in the application private folder, in Symbian Qt implementation
    _db.setDatabaseName("storage.sqlite");
    #endif

    // Open databasee
    if (_db.open()){
        qDebug() << "Settings: database opened";
    }else{
        qWarning()<< "Settings: open database failed" << _db.lastError();;
        return false;
    }
    return true;
}
bool Storage::createTables(){
    QStringList tables = _db.tables();

    if (!tables.contains("tracks")){
        qDebug()<< "Settings: creating tracks table";

        QString sql("CREATE TABLE `tracks`");
        sql.append("(").append( "`id`   int NOT NULL");
        sql.append(",").append( "`name` varchar(255) NOT NULL");
        sql.append(",").append( "`open` tinyint(1) NOT NULL");
        sql.append(",").append( "`creation_time` datetime NOT NULL");
        sql.append(");");

        QSqlQuery q = _db.exec(sql);
        if (q.lastError().isValid()){
            qWarning() << "Storage: creating tracks table failed" << q.lastError();
            return false;
        }
    }

    if (!tables.contains("nodes")){
        qDebug()<< "Settings: creating nodes table";

        QString sql("CREATE TABLE `nodes`");
        sql.append("(").append( "`track_id`   int NOT NULL");
        sql.append(",").append( "`timestamp` datetime NOT NULL");
        sql.append(",").append( "`latitude` double NOT NULL");
        sql.append(",").append( "`longitude` double NOT NULL");
        sql.append(",").append( "`altitude` double NULL ");
        sql.append(",").append( "`horiz_accuracy` double NULL ");
        sql.append(",").append( "`vert_accuracy` double NULL ");
        sql.append(",").append( "`distance_from_prev` double NOT NULL");
        sql.append(",").append( "`ground_speed` double NULL");
        sql.append(");");

        // TODO: what satelites and compas?

        QSqlQuery q = _db.exec(sql);
        if (q.lastError().isValid()){
            qWarning() << "Storage: creating nodes table failed" << q.lastError();
            return false;
        }
    }

    if (!tables.contains("waypoints")){
        qDebug()<< "Settings: creating waypoints table";

        QString sql("CREATE TABLE `waypoints`");
        sql.append("(").append( "`track_id`   int NOT NULL");
        sql.append(",").append( "`timestamp` datetime NOT NULL");
        sql.append(",").append( "`latitude` double NOT NULL");
        sql.append(",").append( "`longitude` double NOT NULL");
        sql.append(",").append( "`title` varchar(255) NOT NULL ");
        sql.append(",").append( "`description` varchar(255) NULL ");
        sql.append(");");

        QSqlQuery q = _db.exec(sql);
        if (q.lastError().isValid()){
            qWarning() << "Storage: creating waypoints table failed" << q.lastError();
            return false;
        }
    }

    return true;
}

bool Storage::closeTrack(int id){

    QSqlQuery sql(_db);
    sql.prepare("UPDATE tracks SET open = 0 WHERE id = :id");
    sql.bindValue(":id", id);
    sql.exec();

    qDebug()<< "Storage: " << sql.executedQuery() << sql.boundValues();

    if (sql.lastError().isValid()){
        qWarning() << "Storage: close track failed" << sql.lastError();
        return false;
    }
    return true;
}

bool Storage::deleteTrack(int id){
    Track *t = getTrack(id);
    if (t == NULL)
        return false;

    QSqlQuery sql(_db);
    sql.prepare("DELETE FROM tracks WHERE id = :id");
    sql.bindValue(":id", id);
    sql.exec();

    qDebug()<< "Storage: " << sql.executedQuery() << sql.boundValues();

    if (sql.lastError().isValid()){
        qWarning() << "Storage: deleting track failed" << sql.lastError();
        return false;
    }

    //QSqlQuery sql2(_db);
    sql.prepare("DELETE FROM nodes WHERE track_id = :id");
    sql.bindValue(":id", id);
    sql.exec();

    qDebug()<< "Storage: " << sql.executedQuery() << sql.boundValues();

    if (sql.lastError().isValid()){
        qWarning() << "Storage: deleting track failed" << sql.lastError();
        return false;
    }

    _allTracks.removeOne(t);
    emit trackDeleted(t);
    t->deleteLater();
    return true;
}

bool Storage::renameTrack(int id, QString newName){
    Track *t = getTrack(id);
    if (t == NULL)
        return false;

    QSqlQuery sql(_db);
    sql.prepare("UPDATE tracks SET name = :name WHERE id = :id");
    sql.bindValue(":name", QVariant(newName));
    sql.bindValue(":id", id);
    sql.exec();

    qDebug()<< "Storage: " << sql.executedQuery() << sql.boundValues();

    if (sql.lastError().isValid()){
        qWarning() << "Storage: deleting track failed" << sql.lastError();
        return false;
    }

    emit trackChanged(t);
    return true;
}

bool Storage::addNode(int trackId, Node node){
    Track *t = getTrack(trackId);
    if (t == NULL)
        return false;

    /*
    const double latitude, const double longitude, const int altitude,
                          const int horizAccuracy, const int vertAccuracy, const double distanceFromPrev,
                          const double groundSpeed
    */

    QString sqlStr = QString("INSERT INTO nodes ");
    sqlStr.append("(track_id,  timestamp,  latitude,  longitude,  altitude,  horiz_accuracy,  vert_accuracy,  distance_from_prev,  ground_speed) ");
    sqlStr.append("VALUES ");
    sqlStr.append("(:track_id, :timestamp, :latitude, :longitude, :altitude, :horiz_accuracy, :vert_accuracy, :distance_from_prev, :ground_speed)");

    QSqlQuery sql(_db);
    sql.prepare(sqlStr);
    sql.bindValue(":track_id", trackId);
    sql.bindValue(":timestamp", node.utcTimestamp);
    sql.bindValue(":latitude", node.latitude);
    sql.bindValue(":longitude", node.longitude);
    sql.bindValue(":altitude", node.altitude);
    sql.bindValue(":horiz_accuracy", node.horizontalAccuracy);
    sql.bindValue(":vert_accuracy", node.verticalAccuracy);
    sql.bindValue(":distance_from_prev", node.distanceFromPrevious);
    sql.bindValue(":ground_speed", node.groundSpeed);
    sql.exec();

    //qDebug()<< "Storage: " << sql.executedQuery() << sql.boundValues();

    if (sql.lastError().isValid()){
        qDebug()<< "Storage: " << sql.executedQuery() << sql.boundValues();
        qWarning() << "Storage: add node failed" << sql.lastError();
        return false;
    }

    emit trackChanged(t);
    return true;
}

QList<Node> Storage::loadNodes(int trackId){
    QList<Node> result;

    QString sqlStr = QString("SELECT * FROM `nodes` WHERE track_id = :track_id ORDER BY timestamp;");
    QSqlQuery q(_db);
    q.prepare(sqlStr);
    q.bindValue(":track_id", trackId);

    q.exec();
    if (!q.lastError().isValid()){
        while (q.next()){
            result.append(Node(q.record()));
        }
    }else{
        qWarning() << "Storage: failed to load nodes " << q.lastError();
    }

    return result;
}
