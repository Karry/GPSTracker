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

#include <QVariant>
#include <QByteArray>

#include "tracksmodel.h"

TracksModel::TracksModel(Storage *storage):
    _storage(storage)
{
    QHash<int, QByteArray> roles;

    roles[ItemRole] = "item";
    roles[IdRole] = "id";
    roles[NameRole] = "name";

    _tracks =  _storage->getAllTracks();
    connect(_storage, SIGNAL(trackCreated(Track*)), this, SLOT(onTrackCreated(Track *)));
    connect(_storage, SIGNAL(trackDeleted(Track*)), this, SLOT(onTrackDeleted(Track *)));
    connect(_storage, SIGNAL(trackChanged(Track*)), this, SLOT(onTrackChanged(Track *)));

    setRoleNames(roles);
}


TracksModel::~TracksModel(){

}

void TracksModel::onTrackCreated(Track *track){
    this->beginInsertRows(QModelIndex(), 0, 0);
    _tracks.prepend(track);
    this->endInsertRows();
}

void TracksModel::onTrackDeleted(Track *track){
    int index = _tracks.indexOf(track);
    if (index<0)
        return;

    this->beginRemoveRows(QModelIndex(), index, index);
    _tracks.removeAt(index);
    this->endRemoveRows();
}

void TracksModel::onTrackChanged(Track *track){
    int index = _tracks.indexOf(track);
    if (index<0)
        return;

    emit dataChanged( createIndex(index, 0, 0), createIndex(index, 0, 0) ) ;
}

int TracksModel::rowCount(const QModelIndex &parent ) const {
    if (parent == QModelIndex()){
        return _tracks.size();
    }
    return 0;
}

QVariant TracksModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
      return QVariant();
    }
    if (index.row() >= 0 && index.row() < _tracks.size()){
        Track *track = _tracks.at(index.row());
        switch (role){
        case ItemRole:
            return QVariant::fromValue(track);
        case IdRole:
            return QVariant(track->getId());
        case NameRole:
            return QVariant(track->getName());
        }
    }
    return QVariant();
}
