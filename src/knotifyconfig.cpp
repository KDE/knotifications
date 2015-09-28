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

#include "knotifyconfig.h"

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <QCache>
#include <QDataStream>
#include <QStandardPaths>

typedef QCache<QString, KSharedConfig::Ptr> ConfigCache;
Q_GLOBAL_STATIC_WITH_ARGS(ConfigCache , static_cache, (15))

static KSharedConfig::Ptr retrieve_from_cache(const QString &filename, QStandardPaths::StandardLocation type = QStandardPaths::GenericConfigLocation)
{
    QCache<QString, KSharedConfig::Ptr> &cache = *static_cache;
    if (cache.contains(filename)) {
        return *cache[filename];
    }

    KSharedConfig::Ptr m = KSharedConfig::openConfig(filename, KConfig::NoGlobals, type);
    cache.insert(filename, new KSharedConfig::Ptr(m));

    return m;
}

void KNotifyConfig::reparseConfiguration()
{
    QCache<QString, KSharedConfig::Ptr> &cache = *static_cache;
    Q_FOREACH (const QString &filename, cache.keys()) {
        (*cache[filename])->reparseConfiguration();
    }
}

void KNotifyConfig::reparseSingleConfiguration(const QString &app)
{
    QCache<QString, KSharedConfig::Ptr> &cache = *static_cache;
    const QString appCacheKey = app + QStringLiteral(".notifyrc");
    if (cache.contains(appCacheKey)) {
        (*cache[appCacheKey])->reparseConfiguration();
    }
}

KNotifyConfig::KNotifyConfig(const QString &_appname, const ContextList &_contexts, const QString &_eventid)
    : appname (_appname),
      contexts(_contexts),
      eventid(_eventid)
{
    eventsfile = retrieve_from_cache(QStringLiteral("knotifications5/") + _appname + QStringLiteral(".notifyrc"), QStandardPaths::GenericDataLocation);
    configfile = retrieve_from_cache(_appname + QStringLiteral(".notifyrc"));
}

KNotifyConfig::~KNotifyConfig()
{
}

KNotifyConfig *KNotifyConfig::copy() const
{
    KNotifyConfig *config = new KNotifyConfig(appname, contexts, eventid);
//     config->title      = title;
//     config->text       = text;
//     config->image      = KNotifyImage(image.data);
//     config->timeout    = timeout;
//     config->winId      = winId;
//     config->actions    = actions;
    config->eventsfile = eventsfile;
    config->configfile = configfile;
    // appname, contexts, eventid already done in constructor

    return config;
}

QString KNotifyConfig::readEntry(const QString &entry, bool path)
{
    QPair<QString, QString> context;

    Q_FOREACH (context, contexts) {
        const QString group = "Event/" + eventid + '/' + context.first + '/' + context.second;

        if (configfile->hasGroup(group)) {
            KConfigGroup cg(configfile, group);
            QString p = path ? cg.readPathEntry(entry, QString()) : cg.readEntry(entry, QString());

            if (!p.isNull()) {
                return p;
            }
        }

        if (eventsfile->hasGroup(group)) {
            KConfigGroup cg(eventsfile, group);
            QString p = path ? cg.readPathEntry(entry, QString()) : cg.readEntry(entry, QString());

            if (!p.isNull()) {
                return p;
            }
        }
    }
//    kDebug() << entry << " not found in contexts ";
    const QString group = "Event/" + eventid;

    if (configfile->hasGroup(group)) {
        KConfigGroup cg(configfile, group);
        QString p = path ? cg.readPathEntry(entry, QString()) : cg.readEntry(entry, QString());

        if (!p.isNull()) {
            return p;
        }
    }
//    kDebug() << entry << " not found in config ";
    if (eventsfile->hasGroup(group)) {
        KConfigGroup cg(eventsfile, group);
        QString p = path ? cg.readPathEntry(entry, QString()) : cg.readEntry(entry, QString());

        if (!p.isNull()) {
            return p;
        }
    }
//    kDebug() << entry << " not found !!! ";

    return QString();
}

QImage KNotifyImage::toImage()
{
    if (dirty) {
        if (source.size() > 4) { // no way an image can fit in less than 4 bytes
            image.loadFromData(source);
        }
        dirty = false;
    }
    return image;
}
