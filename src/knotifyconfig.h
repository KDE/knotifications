/*
   Copyright (C) 2005-2009 by Olivier Goffart <ogoffart at kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef KNOTIFYCONFIG_H
#define KNOTIFYCONFIG_H

#include <QPair>
#include <QList>
#include <QString>

#include "knotifications_export.h"

typedef QList< QPair<QString,QString> > ContextList;

/**
 * Represent the configuration for an event
 * @author Olivier Goffart <ogoffart@kde.org>
*/
class KNOTIFICATIONS_EXPORT KNotifyConfig
{
public:
    KNotifyConfig(const QString &appname, const ContextList &_contexts , const QString &_eventid);
    KNotifyConfig(const KNotifyConfig& other);
    ~KNotifyConfig();

    KNotifyConfig& operator=(const KNotifyConfig& other);

    /**
        * @return entry from the knotifyrc file
        *
        * This will return the configuration from the user for the given key.
        * It first look into the user config file, and then in the global config file.
        *
        * return a null string if the entry doesn't exist
        */
    QString readEntry(const QString &entry , bool path = false) const;

    /**
        * the name of the application that triggered the notification
        */
    const QString &appName() const;

    /**
        * the name of the notification
        */
    const QString &eventId() const;

    /**
     * Name obtained from notify configuration
     */
    QString appConfigName() const;

    /**
     * the icon name of current event obtained from notify configuration
     */
    QString iconName() const;

    /**
        * reparse the cached configs.  to be used when the config may have changed
        */
    static void reparseConfiguration();

private:
    struct Private;
    Private *const d;
};

#endif
