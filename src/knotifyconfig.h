/*
    SPDX-FileCopyrightText: 2005-2009 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KNOTIFYCONFIG_H
#define KNOTIFYCONFIG_H

#include <KSharedConfig>

#include "knotifications_export.h"
#include <QImage>
#include <QObject> //for Wid
#include <QPair>

typedef QList<QPair<QString, QString>> ContextList;

/**
 * @class KNotifyImage knotifyconfig.h KNotifyConfig
 *
 * An image with lazy loading from the byte array
 */
class KNOTIFICATIONS_EXPORT KNotifyImage
{
public:
    KNotifyImage()
        : dirty(false)
    {
    }
    KNotifyImage(const QByteArray &data)
        : source(data)
        , dirty(true)
    {
    }
    QImage toImage();
    bool isNull()
    {
        return dirty ? source.isEmpty() : image.isNull();
    }
    QByteArray data() const
    {
        return source;
    }

private:
    QByteArray source;
    QImage image;
    bool dirty;
};

/**
 * @class KNotifyConfig knotifyconfig.h KNotifyConfig
 *
 * Represent the configuration for an event
 * @author Olivier Goffart <ogoffart@kde.org>
 */
class KNOTIFICATIONS_EXPORT KNotifyConfig
{
public:
    KNotifyConfig(const QString &appname, const ContextList &_contexts, const QString &_eventid);
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
    QString readEntry(const QString &entry, bool path = false);

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
    KSharedConfig::Ptr eventsfile, configfile;
    ContextList contexts;

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
