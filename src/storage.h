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
#include <QList>


// sql
#include <QSqlDatabase>
#include <QSqlError>

#include "settings.h"
#include "node.h"

#ifndef STORAGE_H
#define STORAGE_H

class Track;

class Storage: public QObject{
    Q_OBJECT
    Q_DISABLE_COPY(Storage)

friend class Track;

public:
    Storage(Settings *settings, QObject * parent);
    virtual ~Storage();
    Track* createNewTrack();
    bool init();
    QList<Track*> getAllTracks();
    Track* getTrack(int trackId);

Q_SIGNALS:
    void trackCreated(Track *);

private:
    Settings *getSettings();
    bool closeTrack(int id);
    bool initDatabase();
    bool createTables();
    void loadInfo();
    bool addNode(int trackId, Node node);
    QList<Node> loadNodes(int trackId);

private :
    QSqlDatabase _db;
    int _lastId;
    QList<Track*> _allTracks;
    Settings *_settings;
};



#endif // STORAGE_H
