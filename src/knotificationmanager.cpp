/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart at kde.org>
   Copyright (C) 2013-2015 Martin Klapetek <mklapetek@kde.org>
   Copyright (C) 2017 Eike Hein <hein@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "knotificationmanager_p.h"
#include "knotification.h"

#include <QHash>
#include <QWidget>
#include <QPointer>
#include <QBuffer>
#include <QFileInfo>
#include <KPluginLoader>
#include <KPluginMetaData>

#ifdef QT_DBUS_LIB
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#endif

#include "knotifyconfig.h"
#include "knotificationplugin.h"

#include "notifybylogfile.h"
#include "notifybytaskbar.h"
#include "notifybyexecute.h"
#ifndef Q_OS_ANDROID
#include "notifybypopup.h"
#include "notifybyportal.h"
#else
#include "notifybyandroid.h"
#endif
#include "debug_p.h"

#if defined(HAVE_CANBERRA)
#include "notifybyaudio_canberra.h"
#elif defined(HAVE_PHONON4QT5)
#include "notifybyaudio_phonon.h"
#endif

#ifdef HAVE_SPEECH
#include "notifybytts.h"
#endif

typedef QHash<QString, QString> Dict;

struct Q_DECL_HIDDEN KNotificationManager::Private {
    QHash<int, KNotification *> notifications;
    QHash<QString, KNotificationPlugin *> notifyPlugins;

    // incremental ids for notifications
    int notifyIdCounter;
    QStringList dirtyConfigCache;
    bool inSandbox = false;
    bool portalDBusServiceExists = false;
};

class KNotificationManagerSingleton
{
public:
    KNotificationManager instance;
};

Q_GLOBAL_STATIC(KNotificationManagerSingleton, s_self)

KNotificationManager *KNotificationManager::self()
{
    return &s_self()->instance;
}

KNotificationManager::KNotificationManager()
    : d(new Private)
{
    d->notifyIdCounter = 0;
    qDeleteAll(d->notifyPlugins);
    d->notifyPlugins.clear();

    if (!qEnvironmentVariableIsEmpty("XDG_RUNTIME_DIR")) {
        const QByteArray runtimeDir = qgetenv("XDG_RUNTIME_DIR");
        if (!runtimeDir.isEmpty()) {
            d->inSandbox = QFileInfo::exists(QFile::decodeName(runtimeDir) + QLatin1String("/flatpak-info"));
        }
    } else if (qEnvironmentVariableIsSet("SNAP")) {
        d->inSandbox = true;
    }

#ifdef QT_DBUS_LIB
    if (d->inSandbox) {
        QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
        d->portalDBusServiceExists = interface->isServiceRegistered(QStringLiteral("org.freedesktop.portal.Desktop"));
    }

    QDBusConnection::sessionBus().connect(QString(),
                                          QStringLiteral("/Config"),
                                          QStringLiteral("org.kde.knotification"),
                                          QStringLiteral("reparseConfiguration"),
                                          this,
                                          SLOT(reparseConfiguration(QString)));
#endif
}

KNotificationManager::~KNotificationManager()
{
    delete d;
}

KNotificationPlugin *KNotificationManager::pluginForAction(const QString &action)
{
    KNotificationPlugin *plugin = d->notifyPlugins.value(action);

    // We already loaded a plugin for this action.
    if (plugin) {
        return plugin;
    }

    auto addPlugin = [this](KNotificationPlugin *plugin) {
        d->notifyPlugins[plugin->optionName()] = plugin;
        connect(plugin, &KNotificationPlugin::finished,
                this, &KNotificationManager::notifyPluginFinished);
        connect(plugin, &KNotificationPlugin::actionInvoked,
                this, &KNotificationManager::notificationActivated);
    };

    // Load plugin.
    // We have a series of built-ins up first, and fall back to trying
    // to instantiate an externally supplied plugin.
    if (action == QLatin1String("Popup")) {
#ifndef Q_OS_ANDROID
            if (d->inSandbox && d->portalDBusServiceExists) {
                plugin = new NotifyByPortal(this);
            } else {
                plugin = new NotifyByPopup(this);
            }
#else
        plugin = new NotifyByAndroid(this);
#endif

        addPlugin(plugin);
    } else if (action == QLatin1String("Taskbar")) {
        plugin = new NotifyByTaskbar(this);
        addPlugin(plugin);
    } else if (action == QLatin1String("Sound")) {
#if defined(HAVE_PHONON4QT5) || defined(HAVE_CANBERRA)
        plugin = new NotifyByAudio(this);
        addPlugin(plugin);
#endif
    } else if (action == QLatin1String("Execute")) {
        plugin = new NotifyByExecute(this);
        addPlugin(plugin);
    } else if (action == QLatin1String("Logfile")) {
        plugin = new NotifyByLogfile(this);
        addPlugin(plugin);
    } else if (action == QLatin1String("TTS")) {
#ifdef HAVE_SPEECH
        plugin = new NotifyByTTS(this);
        addPlugin(plugin);
#endif
    } else {
        bool pluginFound = false;

        QList<QObject*> plugins = KPluginLoader::instantiatePlugins(QStringLiteral("knotification/notifyplugins"),
            [&action, &pluginFound](const KPluginMetaData &data) {
                // KPluginLoader::instantiatePlugins loops over the plugins it
                // found and calls this function to determine whether to
                // instantiate them. We use a `pluginFound` var outside the
                // lambda to break out of the loop once we got a match.
                // The reason we can't just use KPluginLoader::findPlugins,
                // loop over the meta data and instantiate only one plugin
                // is because the X-KDE-KNotification-OptionName field is
                // optional (see TODO note below) and the matching plugin
                // may be among the plugins which don't have it.
                if (pluginFound) {
                    return false;
                }

                const QJsonObject &rawData = data.rawData();

                // This field is new-ish and optional. If it's not set we always
                // instantiate the plugin, unless we already got a match.
                // TODO KF6: Require X-KDE-KNotification-OptionName be set and
                // reject plugins without it.
                if (rawData.contains(QStringLiteral("X-KDE-KNotification-OptionName"))) {
                    if (rawData.value(QStringLiteral("X-KDE-KNotification-OptionName")) == action) {
                        pluginFound = true;
                    } else {
                        return false;
                    }
                }

                return true;
            },
            this);

        for (QObject *pluginObj : qAsConst(plugins)) {
            KNotificationPlugin *notifyPlugin = qobject_cast<KNotificationPlugin*>(pluginObj);

            if (notifyPlugin) {
                // We try to avoid unnecessary instantiations (see above), but
                // when they happen keep the resulting plugins around.
                addPlugin(notifyPlugin);

                // Get ready to return the plugin we got asked for.
                if (notifyPlugin->optionName() == action) {
                    plugin = notifyPlugin;
                }
            } else {
                // Not our/valid plugin, so delete the created object.
                pluginObj->deleteLater();
            }
        }
    }

    return plugin;
}

void KNotificationManager::notifyPluginFinished(KNotification *notification)
{
    if (!notification || !d->notifications.contains(notification->id())) {
        return;
    }

    notification->deref();
}

void KNotificationManager::notificationActivated(int id, int action)
{
    if (d->notifications.contains(id)) {
        qCDebug(LOG_KNOTIFICATIONS) << id << " " << action;
        KNotification *n = d->notifications[id];
        n->activate(action);
        close(id);
    }
}

void KNotificationManager::notificationClosed()
{
    KNotification *notification = qobject_cast<KNotification*>(sender());
    if (!notification) {
        return;
    }
    // We cannot do d->notifications.find(notification->id()); here because the
    // notification->id() is -1 or -2 at this point, so we need to look for value
    for (auto iter = d->notifications.begin(); iter != d->notifications.end(); ++iter) {
        if (iter.value() == notification) {
            d->notifications.erase(iter);
            break;
        }
    }
}

void KNotificationManager::close(int id, bool force)
{
    if (force || d->notifications.contains(id)) {
        KNotification *n = d->notifications.value(id);
        qCDebug(LOG_KNOTIFICATIONS) << "Closing notification" << id;

        // Find plugins that are actually acting on this notification
        // call close() only on those, otherwise each KNotificationPlugin::close()
        // will call finish() which may close-and-delete the KNotification object
        // before it finishes calling close on all the other plugins.
        // For example: Action=Popup is a single actions but there is 5 loaded
        // plugins, calling close() on the second would already close-and-delete
        // the notification
        KNotifyConfig notifyConfig(n->appName(), n->contexts(), n->eventId());
        QString notifyActions = notifyConfig.readEntry(QStringLiteral("Action"));

        const auto listActions = notifyActions.split(QLatin1Char('|'));
        for (const QString &action : listActions) {
            if (!d->notifyPlugins.contains(action)) {
                qCDebug(LOG_KNOTIFICATIONS) << "No plugin for action" << action;
                continue;
            }

            d->notifyPlugins[action]->close(n);
        }
    }
}

int KNotificationManager::notify(KNotification *n)
{
    KNotifyConfig notifyConfig(n->appName(), n->contexts(), n->eventId());

    if (d->dirtyConfigCache.contains(n->appName())) {
        notifyConfig.reparseSingleConfiguration(n->appName());
        d->dirtyConfigCache.removeOne(n->appName());
    }

    const QString notifyActions = notifyConfig.readEntry(QStringLiteral("Action"));

    if (notifyActions.isEmpty() || notifyActions == QLatin1String("None")) {
        // this will cause KNotification closing itself fast
        n->ref();
        n->deref();
        return -1;
    }

    d->notifications.insert(d->notifyIdCounter, n);

    // TODO KF6 d-pointer KNotifyConfig and add this there
    if (n->urgency() == KNotification::DefaultUrgency) {
        const QString urgency = notifyConfig.readEntry(QStringLiteral("Urgency"));
        if (urgency == QLatin1String("Low")) {
            n->setUrgency(KNotification::LowUrgency);
        } else if (urgency == QLatin1String("Normal")) {
            n->setUrgency(KNotification::NormalUrgency);
        } else if (urgency == QLatin1String("High")) {
            n->setUrgency(KNotification::HighUrgency);
        } else if (urgency == QLatin1String("Critical")) {
            n->setUrgency(KNotification::CriticalUrgency);
        }
    }

    const auto actionsList = notifyActions.split(QLatin1Char('|'));
    for (const QString &action : actionsList) {
        KNotificationPlugin *notifyPlugin = pluginForAction(action);

        if (!notifyPlugin) {
            qCDebug(LOG_KNOTIFICATIONS) << "No plugin for action" << action;
            continue;
        }

        n->ref();
        qCDebug(LOG_KNOTIFICATIONS) << "Calling notify on" << notifyPlugin->optionName();
        notifyPlugin->notify(n, &notifyConfig);
    }

    connect(n, &KNotification::closed, this, &KNotificationManager::notificationClosed);
    return d->notifyIdCounter++;
}

void KNotificationManager::update(KNotification *n)
{
    KNotifyConfig notifyConfig(n->appName(), n->contexts(), n->eventId());

    for (KNotificationPlugin *p : qAsConst(d->notifyPlugins)) {
        p->update(n, &notifyConfig);
    }
}

void KNotificationManager::reemit(KNotification *n)
{
    notify(n);
}

void KNotificationManager::reparseConfiguration(const QString &app)
{
    if (!d->dirtyConfigCache.contains(app)) {
        d->dirtyConfigCache << app;
    }
}

#include "moc_knotificationmanager_p.cpp"
