/*
    SPDX-FileCopyrightText: 2005-2009 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "knotifyconfig.h"

#include <KConfigGroup>
#include <QCache>
#include <QStandardPaths>

typedef QCache<QString, KSharedConfig::Ptr> ConfigCache;
Q_GLOBAL_STATIC_WITH_ARGS(ConfigCache, static_cache, (15))

static KSharedConfig::Ptr retrieve_from_cache(const QString &filename, QStandardPaths::StandardLocation type = QStandardPaths::GenericConfigLocation)
{
    QCache<QString, KSharedConfig::Ptr> &cache = *static_cache;
    if (cache.contains(filename)) {
        return *cache[filename];
    }

    KSharedConfig::Ptr m = KSharedConfig::openConfig(filename, KConfig::NoGlobals, type);
    // also search for event config files in qrc resources
    if (type == QStandardPaths::GenericDataLocation) {
        m->addConfigSources({QStringLiteral(":/") + filename});
    }
    cache.insert(filename, new KSharedConfig::Ptr(m));

    return m;
}

void KNotifyConfig::reparseConfiguration()
{
    QCache<QString, KSharedConfig::Ptr> &cache = *static_cache;
    const auto listFiles = cache.keys();
    for (const QString &filename : listFiles) {
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
    : appname(_appname)
    , contexts(_contexts)
    , eventid(_eventid)
{
    eventsfile = retrieve_from_cache(QLatin1String("knotifications5/") + _appname + QLatin1String(".notifyrc"), QStandardPaths::GenericDataLocation);
    configfile = retrieve_from_cache(_appname + QStringLiteral(".notifyrc"));
}

KNotifyConfig::~KNotifyConfig()
{
}

KNotifyConfig *KNotifyConfig::copy() const
{
    KNotifyConfig *config = new KNotifyConfig(appname, contexts, eventid);
    config->eventsfile = eventsfile;
    config->configfile = configfile;
    // appname, contexts, eventid already done in constructor
    return config;
}

QString KNotifyConfig::readEntry(const QString &entry, bool path)
{
    for (const QPair<QString, QString> &context : std::as_const(contexts)) {
        const QString group = QLatin1String("Event/") + eventid + QLatin1Char('/') + context.first + QLatin1Char('/') + context.second;

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
    const QString group = QLatin1String("Event/") + eventid;

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
