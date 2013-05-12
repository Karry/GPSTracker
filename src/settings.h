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

// sql
#include <QSqlDatabase>
#include <QSqlError>

#ifndef SETTINGS_H
#define SETTINGS_H

#define METRIC_UNITS "metric"
#define IMPERIAL_UNITS "imperial"

class Settings: public QObject{
    Q_OBJECT
    Q_DISABLE_COPY(Settings)

    Q_PROPERTY(bool nightViewMode
               READ isNightViewMode
               WRITE setNightViewMode
               NOTIFY nightViewModeChanged)
    Q_PROPERTY(QString units
               READ getUnits
               WRITE setUnits
               NOTIFY unitsChanged)
    Q_PROPERTY(double maximumAccuracy
               READ getMaximumAccuracy
               WRITE setMaximumAccuracy)

public:
    Settings(QObject *parent = 0);
    ~Settings();
    bool init();
    bool store();
    double getMaximumAccuracy();
    Q_INVOKABLE QString formatSmallDistance(int distanceM);
    Q_INVOKABLE QString formatSmallDistance(int distanceM, bool canNegative);
    Q_INVOKABLE QString formatSmallDistance(int distanceM, bool canNegative, bool units);

Q_SIGNALS:
    void nightViewModeChanged(bool mode);
    void unitsChanged(QString units);

private:
    bool isNightViewMode();
    void setNightViewMode(bool n);
    bool initDatabase();
    bool createSettingsTable();
    void setMaximumAccuracy(double d);
    QString getUnits();
    void setUnits(QString units);
    QVariant loadProperty(QString name);

private:
    QSqlDatabase _db;
    bool _nightViewMode;
    double _maximumAccuracy;
    QString _units;
};

#endif // SETTINGS_H
