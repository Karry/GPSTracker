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

#include <QAbstractItemModel>
#include <QAbstractListModel>

#include "storage.h"
#include "track.h"

#ifndef TRACKSMODEL_H
#define TRACKSMODEL_H

class TracksModel :public QAbstractListModel
{

    Q_OBJECT
    Q_DISABLE_COPY(TracksModel)
    Q_ENUMS(Role)

public:

    enum Role {
       // general roles
       ItemRole = Qt::UserRole,
       IdRole,
       NameRole
     };

    TracksModel(Storage *storage);
    virtual ~TracksModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

private
Q_SLOTS:
    void onTrackCreated(Track *track);
    void onTrackDeleted(Track *track);
    void onTrackChanged(Track *track);

private:
    Storage *_storage;
    QList<Track*> _tracks;
};

#endif // TRACKSMODEL_H
