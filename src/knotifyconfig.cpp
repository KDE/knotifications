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
// #include <kdebug.h>
// #include <kglobal.h>
#include <QCache>
#include <QDataStream>
#include <QStandardPaths>
#include <QDebug>

struct KNotifyConfig::Private
{
    QString appName;
    QString eventId;

    /**
        * @internal
        */
    KSharedConfig::Ptr eventsFile,configFile;
    ContextList contexts;

};

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

KNotifyConfig::KNotifyConfig(const QString &_appname, const ContextList &_contexts, const QString &_eventid)
    : d(new Private)
{
    d->appName = _appname;
    d->eventId = _eventid;
    d->eventsFile = retrieve_from_cache(QStringLiteral("knotifications5/") + _appname + QStringLiteral(".notifyrc"), QStandardPaths::GenericDataLocation);
    d->configFile = retrieve_from_cache(_appname + QStringLiteral(".notifyrc"));
    d->contexts = _contexts;
}

KNotifyConfig::KNotifyConfig(const KNotifyConfig &other)
    : d(new Private)
{
    d->appName = other.d->appName;
    d->eventId = other.d->eventId;
    d->eventsFile = other.d->eventsFile;
    d->configFile = other.d->configFile;
    d->contexts = other.d->contexts;
}

KNotifyConfig::~KNotifyConfig()
{
    delete d;
}

KNotifyConfig &KNotifyConfig::operator=(const KNotifyConfig &other)
{
    d->appName = other.d->appName;
    d->eventId = other.d->eventId;
    d->eventsFile = other.d->eventsFile;
    d->configFile = other.d->configFile;
    d->contexts = other.d->contexts;

    return *this;
}


QString KNotifyConfig::appConfigName() const
{
    if (d->eventsFile->hasGroup("Global")) {
        KConfigGroup globalgroup(d->eventsFile, QString("Global"));
        return globalgroup.readEntry("Name", globalgroup.readEntry("Comment", d->appName));
    }

    return d->appName;
}

QString KNotifyConfig::iconName() const
{
    const QString group = "Event/" + d->eventId;
    if (d->eventsFile->hasGroup(group)) {
        KConfigGroup eventGroup(d->eventsFile, group);
        if (eventGroup.hasKey("IconName")) {
            return eventGroup.readEntry("IconName", d->appName);
        }
    }

    if (d->eventsFile->hasGroup("Global")) {
        KConfigGroup globalgroup(d->eventsFile, QString("Global"));
        return globalgroup.readEntry("IconName", d->appName);
    }

    return d->appName;
}

QString KNotifyConfig::readEntry(const QString &entry, bool path) const
{
    QPair<QString, QString> context;

    Q_FOREACH (context, d->contexts) {
        const QString group = "Event/" + d->eventId + '/' + context.first + '/' + context.second;

        if (d->configFile->hasGroup(group)) {
            KConfigGroup cg(d->configFile, group);
            QString p = path ? cg.readPathEntry(entry, QString()) : cg.readEntry(entry, QString());

            if (!p.isNull()) {
                return p;
            }
        }

        if (d->eventsFile->hasGroup(group)) {
            KConfigGroup cg(d->eventsFile, group);
            QString p = path ? cg.readPathEntry(entry, QString()) : cg.readEntry(entry, QString());

            if (!p.isNull()) {
                return p;
            }
        }
    }
//    kDebug() << entry << " not found in contexts ";
    const QString group = "Event/" + d->eventId;

    if (d->configFile->hasGroup(group)) {
        KConfigGroup cg(d->configFile, group);
        QString p = path ? cg.readPathEntry(entry, QString()) : cg.readEntry(entry, QString());

        if (!p.isNull()) {
            return p;
        }
    }
//    kDebug() << entry << " not found in config ";
    if (d->eventsFile->hasGroup(group)) {
        KConfigGroup cg(d->eventsFile, group);
        QString p = path ? cg.readPathEntry(entry, QString()) : cg.readEntry(entry, QString());

        if (!p.isNull()) {
            return p;
        }
    }
//    kDebug() << entry << " not found !!! ";

    return QString();
}


const QString &KNotifyConfig::appName() const
{
    return d->appName;
}

const QString &KNotifyConfig::eventId() const
{
    return d->eventId;
}

