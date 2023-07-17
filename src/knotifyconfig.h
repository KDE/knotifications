/*
    SPDX-FileCopyrightText: 2005-2009 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KNOTIFYCONFIG_H
#define KNOTIFYCONFIG_H

#include <KSharedConfig>

#include "knotifications_export.h"
#include <QObject> //for Wid
#include <QPair>

/**
 * @class KNotifyConfig knotifyconfig.h KNotifyConfig
 *
 * Represent the configuration for an event
 * @author Olivier Goffart <ogoffart@kde.org>
 */
class KNOTIFICATIONS_EXPORT KNotifyConfig
{
public:
    KNotifyConfig(const QString &appname, const QString &_eventid);
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
    QString readEntry(const QString &entry, bool path = false) const;

    /**
     * the name of the application that triggered the notification
     */
    QString appname;

    /**
     * @internal
     */
    KSharedConfig::Ptr eventsfile, configfile;

    /**
     * the name of the notification
     */
    QString eventid;

    /**
     * reparse the cached configs.  to be used when the config may have changed
     */
    static void reparseConfiguration();

    static void reparseSingleConfiguration(const QString &app);
};

#endif
