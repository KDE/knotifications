/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2013-2015 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2017 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "knotification.h"
#include "knotification_p.h"
#include "knotificationmanager_p.h"

#include <config-knotifications.h>

#include <QFileInfo>
#include <QHash>

#ifdef HAVE_DBUS
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#endif

#include "knotificationplugin.h"
#include "knotificationreplyaction.h"
#include "knotifyconfig.h"

#if defined(Q_OS_ANDROID)
#include "notifybyandroid.h"
#elif defined(Q_OS_MACOS)
#include "notifybymacosnotificationcenter.h"
#elif defined(WITH_SNORETOAST)
#include "notifybysnore.h"
#elif defined(HAVE_DBUS)
#include "notifybypopup.h"
#include "notifybyportal.h"
#endif
#include "debug_p.h"

typedef QHash<QString, QString> Dict;

struct Q_DECL_HIDDEN KNotificationManager::Private {
    QHash<int, KNotification *> notifications;
    KNotificationPlugin *plugin = nullptr;

    QStringList dirtyConfigCache;
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
#ifdef HAVE_DBUS
    if (isInsideSandbox()) {
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
    delete d->plugin;
}

void KNotificationManager::notifyPluginFinished(KNotification *notification)
{
    if (!notification || !d->notifications.contains(notification->id())) {
        return;
    }

    notification->deref();
}

void KNotificationManager::notificationActivated(int id, const QString &actionId)
{
    if (KNotification *n = d->notifications.value(id)) {
        qCDebug(LOG_KNOTIFICATIONS) << id << " " << actionId;
        n->activate(actionId);
    }

    // we must look up again, as n->activate goes into application code where anything can happen
    if (KNotification *n = d->notifications.value(id)) {
        // Resident actions delegate control over notification lifetime to the client
        if (!n->hints().value(QStringLiteral("resident")).toBool()) {
            close(id);
        }
    }
}

void KNotificationManager::xdgActivationTokenReceived(int id, const QString &token)
{
    KNotification *n = d->notifications.value(id);
    if (n) {
        qCDebug(LOG_KNOTIFICATIONS) << "Token received for" << id << token;
        n->d->xdgActivationToken = token;
        Q_EMIT n->xdgActivationTokenChanged();
    }
}

void KNotificationManager::notificationReplied(int id, const QString &text)
{
    if (KNotification *n = d->notifications.value(id)) {
        if (auto *replyAction = n->replyAction()) {
            // cannot really send out a "activate inline-reply" signal from plugin to manager
            // so we instead assume empty reply is not supported and means normal invocation
            if (text.isEmpty() && replyAction->fallbackBehavior() == KNotificationReplyAction::FallbackBehavior::UseRegularAction) {
                Q_EMIT replyAction->activated();
            } else {
                Q_EMIT replyAction->replied(text);
            }
            close(id);
        }
    }
}

void KNotificationManager::notificationClosed()
{
    KNotification *notification = qobject_cast<KNotification *>(sender());
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

void KNotificationManager::close(int id)
{
    if (d->notifications.contains(id)) {
        KNotification *n = d->notifications.value(id);
        qCDebug(LOG_KNOTIFICATIONS) << "Closing notification" << id;

        Q_ASSERT(d->plugin);
        d->plugin->close(n);
    }
}

void KNotificationManager::notify(KNotification *n)
{
    KNotifyConfig notifyConfig(n->appName(), n->eventId());

    if (d->dirtyConfigCache.contains(n->appName())) {
        notifyConfig.reparseSingleConfiguration(n->appName());
        d->dirtyConfigCache.removeOne(n->appName());
    }

    if (!notifyConfig.isValid()) {
        qCWarning(LOG_KNOTIFICATIONS) << "No event config could be found for event id" << n->eventId() << "under notifyrc file for app" << n->appName();
    }

    const QString notifyActions = notifyConfig.readEntry(QStringLiteral("Action"));

    if (notifyActions.isEmpty() || notifyActions == QLatin1String("None")) {
        // this will cause KNotification closing itself fast
        n->ref();
        n->deref();
        return;
    }

    d->notifications.insert(n->id(), n);

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
        n->d->needUpdate = false;
    }

    n->ref();

    if (!d->plugin) {
#if defined(Q_OS_ANDROID)
        d->plugin = new NotifyByAndroid(this);
#elif defined(WITH_SNORETOAST)
        d->plugin = new NotifyBySnore(this);
#elif defined(Q_OS_MACOS)
        d->plugin = new NotifyByMacOSNotificationCenter(this);
#elif defined(HAVE_DBUS)
        if (d->portalDBusServiceExists) {
            d->plugin = new NotifyByPortal(this);
        } else {
            d->plugin = new NotifyByPopup(this);
        }
#endif

        connect(d->plugin, &KNotificationPlugin::finished, this, &KNotificationManager::notifyPluginFinished);
        connect(d->plugin, &KNotificationPlugin::xdgActivationTokenReceived, this, &KNotificationManager::xdgActivationTokenReceived);
        connect(d->plugin, &KNotificationPlugin::actionInvoked, this, &KNotificationManager::notificationActivated);
        connect(d->plugin, &KNotificationPlugin::replied, this, &KNotificationManager::notificationReplied);
    }

    d->plugin->notify(n, notifyConfig);

    connect(n, &KNotification::closed, this, &KNotificationManager::notificationClosed);
}

void KNotificationManager::update(KNotification *n)
{
    Q_ASSERT(d->plugin);

    KNotifyConfig notifyConfig(n->appName(), n->eventId());
    d->plugin->update(n, notifyConfig);
}

void KNotificationManager::reparseConfiguration(const QString &app)
{
    if (!d->dirtyConfigCache.contains(app)) {
        d->dirtyConfigCache << app;
    }
}

bool KNotificationManager::isInsideSandbox()
{
    // logic is taken from KSandbox::isInside()
    static const bool isFlatpak = QFileInfo::exists(QStringLiteral("/.flatpak-info"));
    static const bool isSnap = qEnvironmentVariableIsSet("SNAP");

    return isFlatpak || isSnap;
}

#include "moc_knotificationmanager_p.cpp"
