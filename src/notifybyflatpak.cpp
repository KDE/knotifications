/*
   Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>
   Copyright (C) 2008 by Dmitry Suzdalev <dimsuz@gmail.com>
   Copyright (C) 2014 by Martin Klapetek <mklapetek@kde.org>
   Copyright (C) 2016 Jan Grulich <jgrulich@redhat.com>

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

#include "notifybyflatpak.h"

#include "knotifyconfig.h"
#include "knotification.h"
#include "debug_p.h"

#include <QtDBus/QtDBus>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QDBusError>
#include <QDBusMessage>
#include <QMap>

#include <kconfiggroup.h>

static const char portalDbusServiceName[] = "org.freedesktop.portal.Desktop";
static const char portalDbusInterfaceName[] = "org.freedesktop.portal.Notification";
static const char portalDbusPath[] = "/org/freedesktop/portal/desktop";

class NotifyByFlatpakPrivate {
public:
    NotifyByFlatpakPrivate(NotifyByFlatpak *parent) : dbusServiceExists(false), q(parent) {}

    /**
     * Sends notification to DBus "org.freedesktop.notifications" interface.
     * @param id knotify-sid identifier of notification
     * @param config notification data
     * @param update If true, will request the DBus service to update
                     the notification with new data from \c notification
     *               Otherwise will put new notification on screen
     * @return true for success or false if there was an error.
     */
    bool sendNotificationToPortal(KNotification *notification, const KNotifyConfig &config);

    /**
     * Sends request to close Notification with id to DBus "org.freedesktop.notifications" interface
     *  @param id knotify-side notification ID to close
     */

    void closePortalNotification(KNotification *notification);
    /**
     * Find the caption and the icon name of the application
     */

    void getAppCaptionAndIconName(const KNotifyConfig &config, QString *appCaption, QString *iconName);

    /**
     * Specifies if DBus Notifications interface exists on session bus
     */
    bool dbusServiceExists;

    /*
     * As we communicate with the notification server over dbus
     * we use only ids, this is for fast KNotifications lookup
     */
    QHash<uint, QPointer<KNotification>> flatpakNotifications;

    /*
     * Holds the id that will be assigned to the next notification source
     * that will be created
     */
    uint nextId;

    NotifyByFlatpak * const q;
};

//---------------------------------------------------------------------------------------

NotifyByFlatpak::NotifyByFlatpak(QObject *parent)
  : KNotificationPlugin(parent),
    d(new NotifyByFlatpakPrivate(this))
{
    // check if service already exists on plugin instantiation
    QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
    d->dbusServiceExists = interface && interface->isServiceRegistered(portalDbusServiceName);

    if (d->dbusServiceExists) {
        onServiceOwnerChanged(portalDbusServiceName, QString(), QStringLiteral("_")); //connect signals
    }

    // to catch register/unregister events from service in runtime
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(this);
    watcher->setConnection(QDBusConnection::sessionBus());
    watcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    watcher->addWatchedService(portalDbusServiceName);
    connect(watcher,&QDBusServiceWatcher::serviceOwnerChanged, this, &NotifyByFlatpak::onServiceOwnerChanged);
}


NotifyByFlatpak::~NotifyByFlatpak()
{
    delete d;
}

void NotifyByFlatpak::notify(KNotification *notification, KNotifyConfig *notifyConfig)
{
    notify(notification, *notifyConfig);
}

void NotifyByFlatpak::notify(KNotification *notification, const KNotifyConfig &notifyConfig)
{
    if (d->flatpakNotifications.contains(notification->id())) {
        // notification is already on the screen, do nothing
        finish(notification);
        return;
    }

    // check if Notifications DBus service exists on bus, use it if it does
    if (d->dbusServiceExists) {
        if (!d->sendNotificationToPortal(notification, notifyConfig)) {
            finish(notification); //an error ocurred.
        }
    }
}

void NotifyByFlatpak::close(KNotification *notification)
{
    if (d->dbusServiceExists) {
        d->closePortalNotification(notification);
    }
}

void NotifyByFlatpak::update(KNotification *notification, KNotifyConfig *notifyConfig)
{
    // TODO not supported by portals
    Q_UNUSED(notification);
    Q_UNUSED(notifyConfig);
}

void NotifyByFlatpak::onServiceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(serviceName);
    // close all notifications we currently hold reference to
    Q_FOREACH (KNotification *n, d->flatpakNotifications) {
        if (n) {
            emit finished(n);
        }
    }

    d->flatpakNotifications.clear();

    if (newOwner.isEmpty()) {
        d->dbusServiceExists = false;
    } else if (oldOwner.isEmpty()) {
        d->dbusServiceExists = true;
        d->nextId = 1;

        // connect to action invocation signals
        bool connected = QDBusConnection::sessionBus().connect(QString(), // from any service
                                                               portalDbusPath,
                                                               portalDbusInterfaceName,
                                                               QStringLiteral("ActionInvoked"),
                                                               this,
                                                               SLOT(onPortalNotificationActionInvoked(QString,QString,QVariantList)));
        if (!connected) {
            qCWarning(LOG_KNOTIFICATIONS) << "warning: failed to connect to ActionInvoked dbus signal";
        }
    }
}

void NotifyByFlatpak::onPortalNotificationActionInvoked(const QString &id, const QString &action, const QVariantList &parameter)
{
    Q_UNUSED(parameter);

    auto iter = d->flatpakNotifications.find(id.toUInt());
    if (iter == d->flatpakNotifications.end()) {
        return;
    }

    KNotification *n = *iter;
    if (n) {
        emit actionInvoked(n->id(), action.toUInt());
    } else {
        d->flatpakNotifications.erase(iter);
    }
}

void NotifyByFlatpakPrivate::getAppCaptionAndIconName(const KNotifyConfig &notifyConfig, QString *appCaption, QString *iconName)
{
    KConfigGroup globalgroup(&(*notifyConfig.eventsfile), QStringLiteral("Global"));
    *appCaption = globalgroup.readEntry("Name", globalgroup.readEntry("Comment", notifyConfig.appname));

    KConfigGroup eventGroup(&(*notifyConfig.eventsfile), QStringLiteral("Event/%1").arg(notifyConfig.eventid));
    if (eventGroup.hasKey("IconName")) {
        *iconName = eventGroup.readEntry("IconName", notifyConfig.appname);
    } else {
        *iconName = globalgroup.readEntry("IconName", notifyConfig.appname);
    }
}

bool NotifyByFlatpakPrivate::sendNotificationToPortal(KNotification *notification, const KNotifyConfig &notifyConfig_nocheck)
{
    QDBusMessage dbusNotificationMessage;
    dbusNotificationMessage = QDBusMessage::createMethodCall(portalDbusServiceName,
                                                             portalDbusPath,
                                                             portalDbusInterfaceName,
                                                             QStringLiteral("AddNotification"));

    QVariantList args;
    // Will be used only with flatpak portal
    QVariantMap portalArgs;

    QString appCaption;
    QString iconName;
    getAppCaptionAndIconName(notifyConfig_nocheck, &appCaption, &iconName);

    //did the user override the icon name?
    if (!notification->iconName().isEmpty()) {
        iconName = notification->iconName();
    }

    QString title = notification->title().isEmpty() ? appCaption : notification->title();
    QString text = notification->text();

    // galago spec defines action list to be list like
    // (act_id1, action1, act_id2, action2, ...)
    //
    // assign id's to actions like it's done in fillPopup() method
    // (i.e. starting from 1)
    QList<QVariantMap> buttons;
    buttons.reserve(notification->actions().count());

    int actId = 0;
    Q_FOREACH (const QString &actionName, notification->actions()) {
        actId++;
        QVariantMap button = {
            {QStringLiteral("action"), QString::number(actId)},
            {QStringLiteral("label"), actionName}
        };
        buttons << button;
    }

    qDBusRegisterMetaType<QList<QVariantMap> >();
    portalArgs.insert(QStringLiteral("icon"), iconName);
    portalArgs.insert(QStringLiteral("title"), title);
    portalArgs.insert(QStringLiteral("body"), text);
    portalArgs.insert(QStringLiteral("buttons"), QVariant::fromValue<QList<QVariantMap> >(buttons));

    args.append(QString::number(nextId));
    args.append(portalArgs);

    dbusNotificationMessage.setArguments(args);

    QDBusPendingCall notificationCall = QDBusConnection::sessionBus().asyncCall(dbusNotificationMessage, -1);

    // If we are in sandbox we don't need to wait for returned notification id
    flatpakNotifications.insert(nextId++, notification);

    return true;
}

void NotifyByFlatpakPrivate::closePortalNotification(KNotification *notification)
{
    uint id = flatpakNotifications.key(notification, 0);

    qCDebug(LOG_KNOTIFICATIONS) << "ID: " << id;

    if (id == 0) {
        qCDebug(LOG_KNOTIFICATIONS) << "not found dbus id to close" << notification->id();
        return;
    }

    QDBusMessage m = QDBusMessage::createMethodCall(portalDbusServiceName,
                                                    portalDbusPath,
                                                    portalDbusInterfaceName,
                                                    QStringLiteral("RemoveNotification"));
    m.setArguments({QString::number(id)});

    // send(..) does not block
    bool queued = QDBusConnection::sessionBus().send(m);

    if (!queued) {
        qCWarning(LOG_KNOTIFICATIONS) << "Failed to queue dbus message for closing a notification";
    }
}
