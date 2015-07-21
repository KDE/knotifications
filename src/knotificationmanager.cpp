/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart at kde.org>

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

#include <QDebug>
#include <QHash>
#include <QWidget>
#include <QDBusConnection>
#include <QPointer>
#include <QBuffer>
#include <KPluginLoader>

#include "knotifyconfig.h"
#include "knotificationplugin.h"
#include "notifybypopup.h"
#include "notifybyaudio.h"
#include "notifybylogfile.h"
#include "notifybytaskbar.h"
#include "notifybyexecute.h"

#ifdef HAVE_SPEECH
#include "notifybytts.h"
#endif

typedef QHash<QString, QString> Dict;

struct KNotificationManager::Private {
    QHash<int, KNotification *> notifications;
    QHash<QString, KNotificationPlugin *> notifyPlugins;

    // incremental ids for notifications
    int notifyIdCounter;
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
    addPlugin(new NotifyByAudio(this));
    //TODO reactivate on Mac/Win when KWindowSystem::demandAttention will implemented on this system.
    #ifndef Q_OS_MAC
    addPlugin(new NotifyByTaskbar(this));
    #endif

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
        qDebug() << id << " " << action;
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
        KNotification *n = d->notifications.take(id);
        qDebug() << "Closing notification" << id;

        Q_FOREACH (KNotificationPlugin *plugin, d->notifyPlugins) {
            plugin->close(n);
        }
    }
}

int KNotificationManager::notify(KNotification *n)
{
    KNotifyConfig notifyConfig(n->appName(), n->contexts(), n->eventId());

    QString notifyActions = notifyConfig.readEntry("Action");

    if (notifyActions.isEmpty() || notifyActions == QLatin1String("None")) {
        // this will cause KNotification closing itself fast
        n->deref();
        return -1;
    }

    d->notifications.insert(++d->notifyIdCounter, n);

    Q_FOREACH (const QString &action, notifyActions.split('|')) {
        if (!d->notifyPlugins.contains(action)) {
            qDebug() << "No plugin for action" << action;
            continue;
        }

        KNotificationPlugin *notifyPlugin = d->notifyPlugins[action];
        n->ref();
        qDebug() << "Calling notify on" << notifyPlugin->optionName();
        notifyPlugin->notify(n, &notifyConfig);
    }

    connect(n, &KNotification::closed, this, &KNotificationManager::notificationClosed);
    return d->notifyIdCounter;
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

#include "moc_knotificationmanager_p.cpp"
