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

#include <QTextCodec>
#include <QApplication>
#include <QDebug>
#include <QLocale>

#include "qmlapplicationviewer.h"
#include "gpstracker.h"

Q_DECL_EXPORT int main(int argc, char *argv[]){
    // Use utf8 for QString (Without this, some special characters in QString from C++ are not correctly displayed in QML)
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));

    QScopedPointer<QApplication> app(createApplication(argc, argv));

    GpsTracker *core = new GpsTracker(app.data());

    if (!core->init()){
        qWarning() << "GpsTracker initializing failed, exiting";
        return -1;
    }

    return app->exec();
}
