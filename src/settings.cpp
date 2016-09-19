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
#include <QStringList>
#include <QMetaProperty>
#include <QDir>

// sql
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include <math.h>       /* floor */

#include "settings.h"



Settings::Settings(QObject *parent):
    QObject(parent),
    _maximumAccuracy(30),
    _units(QLocale::system().measurementSystem() == QLocale::MetricSystem ? UNITS_METRIC: UNITS_IMPERIAL),
    _posFormat(POS_FORMAT_DEGREES){

}

Settings::~Settings(){
    qDebug() << "Settings: destroing";
    _db.close();
    QSqlDatabase::removeDatabase("settings");
}

bool Settings::init(){
    qDebug()<< "Settings: load";

    if (!this->initDatabase())
        return false;

    if (!createSettingsTable())
       return false;

    const QMetaObject *metaobject = this->metaObject();
    int count = metaobject->propertyCount();
    for (int i=0; i<count; ++i) {
       QMetaProperty metaproperty = metaobject->property(i);
       const char *name = metaproperty.name();
       QVariant v = loadProperty(name);
       qDebug()<< "Settings: set" << name << "to" << v;
       this->setProperty(name, v);
    }

    return true;
}


bool Settings::initDatabase(){
    // Find QSLite driver
    this->_db = QSqlDatabase::addDatabase("QSQLITE", "settings");
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

    path.append(QDir::separator()).append("settings.sqlite");
    path = QDir::toNativeSeparators(path);
    _db.setDatabaseName(path);
    #else
    // NOTE: File exists in the application private folder, in Symbian Qt implementation
    _db.setDatabaseName("settings.sqlite");
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

bool Settings::createSettingsTable(){
    QStringList tables = _db.tables();
    if (tables.contains("settings"))
        return true;

    qDebug()<< "Settings: creating settings table";

    QString sql("CREATE TABLE `settings`");
    sql.append("(").append( "`property` varchar(255) NOT NULL");
    sql.append(",").append( "`value`    varchar(255) NOT NULL");
    sql.append(");");

    QSqlQuery q = _db.exec(sql);
    if (q.lastError().isValid()){
        qWarning() << "Settings: creating settings table failed" << q.lastError();
        return false;
    }

    return true;
}

QVariant Settings::loadProperty(QString name){
    QString sql("SELECT `value` FROM `settings` ");
    sql.append("WHERE `property` LIKE '").append(name).append("';");

    QSqlQuery q = _db.exec(sql);
    if (q.lastError().isValid()){
        qWarning()<< "Settings: sql "<< sql<< q.lastError();
        return QVariant();
    }
    if (q.next())
        return q.value(0);

    return QVariant();
}

bool Settings::store(){
    qDebug()<< "Settings: store";

    if (!_db.isOpen()){
        qWarning()<< "Settings: db is not open";
        return false;
    }


    const QMetaObject *metaobject = this->metaObject();
    int count = metaobject->propertyCount();
    if (count == 0)
        return true;

    //QString sql("INSERT INTO `settings` (`property`, `value`) VALUES ");
    QString sql("INSERT INTO `settings` ");
    QString deleteSql("DELETE FROM `settings` WHERE `property` IN (");
    for (int i=0; i<count; ++i) {
        QMetaProperty metaproperty = metaobject->property(i);
        const char *name = metaproperty.name();
        QVariant v = this->property(name);
        if (i!=0){
            sql.append(" UNION ALL ");
            deleteSql.append(",");
        }
        sql.append("SELECT '").append(name).append("','")
                .append(v.toString()).append("'");
        deleteSql.append("'").append(name).append("'");
    }
    sql.append(";");
    deleteSql.append(");");

    qDebug()<< "Settings:"<<deleteSql;
    QSqlQuery q = _db.exec(deleteSql);
    if (q.lastError().isValid()){
        qWarning() << "Settings: query failed: " << deleteSql
                   <<"("<<q.lastError()<<")";
        return false;
    }


    qDebug()<< "Settings:"<<sql;
    q = _db.exec(sql);
    if (q.lastError().isValid()){
        qWarning() << "Settings: query failed: " << sql
                   <<"("<<q.lastError()<<")";
        return false;
    }

    return true;
}

bool Settings::isNightViewMode(){
    return _nightViewMode;
}

void Settings::setNightViewMode(bool n){
    if (_nightViewMode != n){
        _nightViewMode = n;
        store();
        emit nightViewModeChanged(_nightViewMode);
    }
}

double Settings::getMaximumAccuracy(){
    return _maximumAccuracy;
}

void Settings::setMaximumAccuracy(double d){
    _maximumAccuracy = d;
    store();
}

QString Settings::getUnits(){
    return _units;
}

void Settings::setUnits(QString units){
    if (units != UNITS_METRIC &&  units != UNITS_IMPERIAL){
        qWarning() << "Settings: units is not correct (" << units << ")";
        return;
    }
    if (_units != units){
        _units = units;
        store();
        emit unitsChanged(units);
    }
}

QString Settings::getPosFormat(){
   return _posFormat;
}

void Settings::setPosFormat(QString posFormat){
    if (posFormat != POS_FORMAT_DEGREES && posFormat != POS_FORMAT_GEOCACHING && posFormat != POS_FORMAT_NUMERIC){
        qWarning() << "Settings: possition format is not correct (" << posFormat << ")";
        return;
    }
    if (_posFormat != posFormat){
        _posFormat = posFormat;
        store();
        emit posFormatChanged(posFormat);
    }
}

QString Settings::formatSmallDistance(int distanceM, bool canNegative, bool units){
    if ((distanceM < 0) && (!canNegative))
        return "?";

    if (getUnits() == UNITS_IMPERIAL){
        /* FIXME: I'am not sure that it is right */
        return  QString("%1").arg(qRound( (double)distanceM * 3.2808 )) + (units? " ft": "");
    }
    if (getUnits() == UNITS_METRIC){
        return QString("%1").arg(distanceM) + (units ? " m": "");
    }
    return QString("%1").arg(distanceM) + " m";
}

QString Settings::formatDistance(double distanceM, bool canNegative){
    if ((distanceM < 0) && (!canNegative))
        return "?";

    if (_units == UNITS_METRIC){
            double tmp = (distanceM / 1000);
            return (tmp >= 10 ? QString("%1").arg(qRound(tmp)) : toFixed(tmp, 1)) + " km";
    }
    if (_units == UNITS_IMPERIAL){
            /* FIXME: I'am not sure that it is right */
            double tmp = (distanceM / 1609.344);
            return (tmp >= 10 ? QString("%1").arg(qRound(tmp)) : toFixed(tmp, 1)) + " miles";
    }

    return QString("%1").arg(qRound(distanceM)) + " m";
}

QString Settings::formatPosition(double latitude, double longitude){
    return formatLatitude(latitude) + "    " + formatLongitude(longitude);
}

QString Settings::formatLatitude(double degree){
    if (_posFormat == POS_FORMAT_NUMERIC)
        return toFixed(degree, 5);

    if (_posFormat == POS_FORMAT_GEOCACHING)
        return QString(degree > 0? "N" : "S") + " " + formatDegreeLikeGeocaching( qAbs(degree) );

    return formatDegree( qAbs(degree) ) + (degree > 0 ? "N" : "S");
}

QString Settings::formatLongitude(double degree){
    if (_posFormat == POS_FORMAT_NUMERIC)
        return toFixed(degree, 5);

    if (_posFormat == POS_FORMAT_GEOCACHING)
        return QString(degree > 0 ? "E" : "W") + " " + formatDegreeLikeGeocaching( qAbs(degree) );

    return formatDegree( qAbs(degree) ) + (degree > 0 ? "E" : "W");
}

QString Settings::formatDegree(double degree){
    double minutes = (degree - floor(degree)) * 60;
    double seconds = (minutes - floor(minutes )) * 60;
    return QString("%1° ").arg(floor(degree))
        + (minutes < 10 ? "0" : "") + QString("%1'").arg(floor(minutes))
        + (seconds < 10 ? "0" : "") + toFixed(seconds, 2) ;
}

QString Settings::formatDegreeLikeGeocaching(double degree){
    double minutes = (degree - floor(degree)) * 60;
    return QString("%1°").arg(floor(degree))
        + (minutes < 10 ? "0" : "") + toFixed(minutes,4);
}

QString Settings::formatSpeed(double speeedMPS){
    if (speeedMPS < 0)
        return "?";

    if (_units == UNITS_IMPERIAL){
        /* FIXME: I'am not sure that it is right */
        return QString("%1").arg(qRound(speeedMPS * 2.237)) + " MPH";
    }
    if (_units == UNITS_METRIC){
        return QString("%1").arg(qRound(speeedMPS * 3.6)) + " km/h";
    }
    return QString("%1").arg(floor(speeedMPS)) + " m/s";
}

QString Settings::toFixed(double radix, int decimals){
    QLocale loc = QLocale::system(); // current locale
    //return QString("%1").arg(radix, 0, 'f', decimals);
    return loc.toString(radix, 'f', decimals);
}
