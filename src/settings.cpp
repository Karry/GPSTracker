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

#include "settings.h"


Settings::Settings(QObject *parent):
    QObject(parent),
    _maximumAccuracy(30){

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
    _nightViewMode = n;
    store();
    emit nightViewModeChanged(_nightViewMode);
}

double Settings::getMaximumAccuracy(){
    return _maximumAccuracy;
}

void Settings::setMaximumAccuracy(double d){
    _maximumAccuracy = d;
    store();
}
