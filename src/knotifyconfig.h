/*
   Copyright (C) 2005-2009 by Olivier Goffart <ogoffart at kde.org>


   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */

#ifndef KNOTIFYCONFIG_H
#define KNOTIFYCONFIG_H

#include <ksharedconfig.h>

#include <QPair>
#include <QPixmap>
#include <QObject> //for Wid

#include "knotifications_export.h"

typedef QList< QPair<QString,QString> > ContextList;

/**
 * An image with lazy loading from the byte array
 */
class KNOTIFICATIONS_EXPORT KNotifyImage
{
    public:
        KNotifyImage() : dirty(false) {}
        KNotifyImage(const QByteArray &data) : source(data), dirty(true) {}
        QImage toImage();
        bool isNull() {
            return dirty ? source.isEmpty() : image.isNull();
        }
        QByteArray data() const {
            return source;
        }
    private:
        QByteArray source;
        QImage image;
        bool dirty;
};


/**
 * Represent the configuration for an event
 * @author Olivier Goffart <ogoffart@kde.org>
*/
class KNOTIFICATIONS_EXPORT KNotifyConfig
{
public:
    KNotifyConfig(const QString &appname, const ContextList &_contexts , const QString &_eventid);
    ~KNotifyConfig();

    KNotifyConfig *copy() const;

    /**
        * @return entry from the knotifyrc file
        *
        * This will return the configuration from the user for the given key.
        * It first look into the user config file, and then in the global config file.
        *
        * return a null string if the entry doesn't exist
        */
    QString readEntry(const QString &entry , bool path = false);

    /**
        * the pixmap to put on the notification
        */
    KNotifyImage image;

    /**
        * the name of the application that triggered the notification
        */
    QString appname;

    /**
        * @internal
        */
    KSharedConfig::Ptr eventsfile,configfile;
    ContextList contexts;

    /**
        * the name of the notification
        */
    QString eventid;

    /**
        * reparse the cached configs.  to be used when the config may have changed
        */
    static void reparseConfiguration();
};

#endif
