/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart at kde.org>
   Copyright (C) 2013-2015 Martin Klapetek <mklapetek@kde.org>

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
#include <QDBusConnection>
#include <QPointer>
#include <QBuffer>
#include <KPluginLoader>

#include "knotifyconfig.h"
#include "knotificationplugin.h"
#include "notifybypopup.h"

#include "notifybylogfile.h"
#include "notifybytaskbar.h"
#include "notifybyexecute.h"
#include "debug_p.h"

#ifdef HAVE_PHONON4QT5
#include "notifybyaudio.h"
#endif

#ifdef HAVE_SPEECH
#include "notifybytts.h"
#endif

typedef QHash<QString, QString> Dict;

struct KNotificationManager::Private {
    QHash<int, KNotification *> notifications;
    QHash<QString, KNotificationPlugin *> notifyPlugins;

    // incremental ids for notifications
    int notifyIdCounter;
    QStringList dirtyConfigCache;
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
    addPlugin(new NotifyByPopup(this));
    addPlugin(new NotifyByExecute(this));
    //FIXME: port and reenable
//     addPlugin(new NotifyByLogfile(this));

#ifdef HAVE_PHONON4QT5
    addPlugin(new NotifyByAudio(this));
#endif

    addPlugin(new NotifyByTaskbar(this));

#ifdef HAVE_SPEECH
    addPlugin(new NotifyByTTS(this));
#endif

    QList<QObject*> plugins = KPluginLoader::instantiatePlugins(QStringLiteral("knotification/notifyplugins"),
                                                                std::function<bool(const KPluginMetaData &)>(),
                                                                this);

    Q_FOREACH (QObject *plugin, plugins) {
        KNotificationPlugin *notifyPlugin = qobject_cast<KNotificationPlugin*>(plugin);
        if (notifyPlugin) {
            addPlugin(notifyPlugin);
        } else {
            // not our/valid plugin, so delete the created object
            plugin->deleteLater();
        }
    }

    QDBusConnection::sessionBus().connect(QString(),
                                          QStringLiteral("/Config"),
                                          QStringLiteral("org.kde.knotification"),
                                          QStringLiteral("reparseConfiguration"),
                                          this,
                                          SLOT(reparseConfiguration(QString)));

}

KNotificationManager::~KNotificationManager()
{
    delete d;
}

void KNotificationManager::addPlugin(KNotificationPlugin *notifyPlugin)
{
    d->notifyPlugins[notifyPlugin->optionName()] = notifyPlugin;
    connect(notifyPlugin, SIGNAL(finished(KNotification*)), this, SLOT(notifyPluginFinished(KNotification*)));
    connect(notifyPlugin, SIGNAL(actionInvoked(int, int)), this, SLOT(notificationActivated(int, int)));
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

        Q_FOREACH (const QString &action, notifyActions.split('|')) {
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

    QString notifyActions = notifyConfig.readEntry(QStringLiteral("Action"));

    if (notifyActions.isEmpty() || notifyActions == QLatin1String("None")) {
        // this will cause KNotification closing itself fast
        n->deref();
        return -1;
    }

    d->notifications.insert(d->notifyIdCounter, n);

    Q_FOREACH (const QString &action, notifyActions.split('|')) {
        if (!d->notifyPlugins.contains(action)) {
            qCDebug(LOG_KNOTIFICATIONS) << "No plugin for action" << action;
            continue;
        }

        KNotificationPlugin *notifyPlugin = d->notifyPlugins[action];
        n->ref();
        qCDebug(LOG_KNOTIFICATIONS) << "Calling notify on" << notifyPlugin->optionName();
        notifyPlugin->notify(n, &notifyConfig);
    }

    connect(n, &KNotification::closed, this, &KNotificationManager::notificationClosed);
    return d->notifyIdCounter++;
}

void KNotificationManager::update(KNotification *n)
{
    QByteArray pixmapData;
    if (!n->pixmap().isNull()) {
        QBuffer buffer(&pixmapData);
        buffer.open(QIODevice::WriteOnly);
        n->pixmap().save(&buffer, "PNG");
    }

    KNotifyConfig notifyConfig(n->appName(), n->contexts(), n->eventId());

    Q_FOREACH (KNotificationPlugin *p, d->notifyPlugins) {
        p->update(n, &notifyConfig);
    }
}

void KNotificationManager::reemit(KNotification *n)
{
    QVariantList contextList;
    typedef QPair<QString, QString> Context;
    foreach (const Context &ctx, n->contexts()) {
        QVariantList vl;
        vl << ctx.first << ctx.second;
        contextList << vl;
    }


    notify(n);
}

void KNotificationManager::reparseConfiguration(const QString &app)
{
    if (!d->dirtyConfigCache.contains(app)) {
        d->dirtyConfigCache << app;
    }
}

#include "moc_knotificationmanager_p.cpp"
