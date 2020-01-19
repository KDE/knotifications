/*
   Copyright (C) 2005-2009 by Olivier Goffart <ogoffart at kde.org>
   Copyright (C) 2008 by Dmitry Suzdalev <dimsuz@gmail.com>
   Copyright (C) 2014 by Martin Klapetek <mklapetek@kde.org>

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

#include "notifybypopup.h"
#include "imageconverter.h"

#include "kpassivepopup.h"
#include "knotifyconfig.h"
#include "knotification.h"
#include "debug_p.h"

#include <QApplication>
#include <QBuffer>
#include <QImage>
#include <QLabel>
#include <QGuiApplication>
#include <QLayout>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QDBusMessage>
#include <QXmlStreamReader>
#include <QMap>
#include <QHash>
#include <QPointer>
#include <QMutableListIterator>
#include <QThread>
#include <QFontMetrics>
#include <QIcon>
#include <QUrl>
#include <QScreen>

#include <kconfiggroup.h>

static const char dbusServiceName[] = "org.freedesktop.Notifications";
static const char dbusInterfaceName[] = "org.freedesktop.Notifications";
static const char dbusPath[] = "/org/freedesktop/Notifications";

class NotifyByPopupPrivate {
public:
    NotifyByPopupPrivate(NotifyByPopup *parent) : q(parent) {}
    /**
     * @internal
     * Fills the KPassivePopup with data
     */
    void fillPopup(KPassivePopup *popup, KNotification *notification, const KNotifyConfig &config);

    /**
     * Sends notification to DBus "org.freedesktop.notifications" interface.
     * @param id knotify-sid identifier of notification
     * @param config notification data
     * @param update If true, will request the DBus service to update
                     the notification with new data from \c notification
     *               Otherwise will put new notification on screen
     * @return true for success or false if there was an error.
     */
    bool sendNotificationToGalagoServer(KNotification *notification, const KNotifyConfig &config, bool update = false);
    /**
     * Sends request to close Notification with id to DBus "org.freedesktop.notifications" interface
     *  @param id knotify-side notification ID to close
     */
    void closeGalagoNotification(KNotification *notification);
    /**
     * Find the caption and the icon name of the application
     */
    void getAppCaptionAndIconName(const KNotifyConfig &config, QString *appCaption, QString *iconName);
    /*
     * Query the dbus server for notification capabilities
     * If no DBus server is present, use fallback capabilities for KPassivePopup
     */
    void queryPopupServerCapabilities();

    // the y coordinate of the next position popup should appears
    int nextPosition;
    int animationTimer;
    /**
     * Specifies if DBus Notifications interface exists on session bus
     */
    bool dbusServiceExists;

    bool dbusServiceActivatable;

    /**
     * DBus notification daemon capabilities cache.
     * Do not use this variable. Use #popupServerCapabilities() instead.
     * @see popupServerCapabilities
     */
    QStringList popupServerCapabilities;

    /**
     * In case we still don't know notification server capabilities,
     * we need to query those first. That's done in an async way
     * so we queue all notifications while waiting for the capabilities
     * to return, then process them from this queue
     */
    QList<QPair<KNotification*, KNotifyConfig> > notificationQueue;
    /**
     * Whether the DBus notification daemon capability cache is up-to-date.
     */
    bool dbusServiceCapCacheDirty;

    /**
     * Keeps the map of notifications done in KPassivePopup
     */
    QMap<KNotification*, KPassivePopup *> passivePopups;

    /*
     * As we communicate with the notification server over dbus
     * we use only ids, this is for fast KNotifications lookup
     */
    QHash<uint, QPointer<KNotification>> galagoNotifications;


    NotifyByPopup * const q;
};

//---------------------------------------------------------------------------------------

NotifyByPopup::NotifyByPopup(QObject *parent)
  : KNotificationPlugin(parent),
    d(new NotifyByPopupPrivate(this))
{
    d->animationTimer = 0;
    d->dbusServiceExists = false;
    d->dbusServiceActivatable = false;
    d->dbusServiceCapCacheDirty = true;
    d->nextPosition = -1;

    // check if service already exists on plugin instantiation
    QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
    d->dbusServiceExists = interface && interface->isServiceRegistered(QString::fromLatin1(dbusServiceName));

    if (d->dbusServiceExists) {
        onServiceOwnerChanged(QString::fromLatin1(dbusServiceName), QString(), QStringLiteral("_")); //connect signals
    }

    // to catch register/unregister events from service in runtime
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(this);
    watcher->setConnection(QDBusConnection::sessionBus());
    watcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    watcher->addWatchedService(QString::fromLatin1(dbusServiceName));
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged,
            this, &NotifyByPopup::onServiceOwnerChanged);

#ifndef Q_WS_WIN
    if (!d->dbusServiceExists) {
        QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.DBus"),
                                                                QStringLiteral("/org/freedesktop/DBus"),
                                                                QStringLiteral("org.freedesktop.DBus"),
                                                                QStringLiteral("ListActivatableNames"));
        QDBusReply<QStringList> reply = QDBusConnection::sessionBus().call(message);
        if (reply.isValid() && reply.value().contains(QString::fromLatin1(dbusServiceName))) {
            d->dbusServiceActivatable = true;
            //if the service is activatable, we can assume it exists even if it is not currently running
            d->dbusServiceExists = true;
        }
    }
#endif
}


NotifyByPopup::~NotifyByPopup()
{
    for (KPassivePopup *p : qAsConst(d->passivePopups)) {
        p->deleteLater();
    }

    delete d;
}

void NotifyByPopup::notify(KNotification *notification, KNotifyConfig *notifyConfig)
{
    notify(notification, *notifyConfig);
}

void NotifyByPopup::notify(KNotification *notification, const KNotifyConfig &notifyConfig)
{
    if (d->passivePopups.contains(notification) || d->galagoNotifications.contains(notification->id())) {
        // notification is already on the screen, do nothing
        finish(notification);
        return;
    }

    // check if Notifications DBus service exists on bus, use it if it does
    if (d->dbusServiceExists) {
        if (d->dbusServiceCapCacheDirty) {
            // if we don't have the server capabilities yet, we need to query for them first;
            // as that is an async dbus operation, we enqueue the notification and process them
            // when we receive dbus reply with the server capabilities
            d->notificationQueue.append(qMakePair(notification, notifyConfig));
            d->queryPopupServerCapabilities();
        } else {
            if (!d->sendNotificationToGalagoServer(notification, notifyConfig)) {
                finish(notification); //an error occurred.
            }
        }
        return;
    }

    // Persistent     => 0  == infinite timeout
    // CloseOnTimeout => -1 == let the server decide
    int timeout = (notification->flags() & KNotification::Persistent) ? 0 : -1;

    // Check if this object lives in the GUI thread and return if it doesn't
    // as Qt cannot create/handle widgets in non-GUI threads
    if (QThread::currentThread() != qApp->thread()) {
        qCWarning(LOG_KNOTIFICATIONS) << "KNotification did not detect any running org.freedesktop.Notifications server and fallback notifications cannot be used from non-GUI thread!";
        return;
    }

    if (!qobject_cast<QApplication *>(QCoreApplication::instance())) {
        qCWarning(LOG_KNOTIFICATIONS) << "KNotification did not detect any running org.freedesktop.Notifications server and fallback notifications cannot be used without a QApplication!";
        return;
    }

    // last fallback - display the popup using KPassivePopup
    KPassivePopup *pop = new KPassivePopup(notification->widget());
    d->passivePopups.insert(notification, pop);
    d->fillPopup(pop, notification, notifyConfig);

    QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
    if (d->nextPosition == -1) {
        d->nextPosition = screen.top();
    }
    pop->setAutoDelete(true);
    connect(pop, &QObject::destroyed, this, &NotifyByPopup::onPassivePopupDestroyed);

    pop->setTimeout(timeout);
    pop->adjustSize();
    pop->show(QPoint(screen.left() + screen.width()/2 - pop->width()/2 , d->nextPosition));
    d->nextPosition += pop->height();
}

void NotifyByPopup::onPassivePopupDestroyed()
{
    const QObject *destroyedPopup = sender();

    if (!destroyedPopup) {
        return;
    }

    for (QMap<KNotification*, KPassivePopup*>::iterator it = d->passivePopups.begin(); it != d->passivePopups.end(); ++it) {
        QObject *popup = it.value();
        if (popup && popup == destroyedPopup) {
            finish(it.key());
            d->passivePopups.remove(it.key());
            break;
        }
    }

    //relocate popup
    if (!d->animationTimer) {
        d->animationTimer = startTimer(10);
    }
}

void NotifyByPopup::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != d->animationTimer) {
        KNotificationPlugin::timerEvent(event);
        return;
    }

    bool cont = false;
    QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
    d->nextPosition = screen.top();

    for (KPassivePopup *popup : qAsConst(d->passivePopups))
    {
        int y = popup->pos().y();
        if (y > d->nextPosition) {
            y = qMax(y - 5, d->nextPosition);
            d->nextPosition = y + popup->height();
            cont = cont || y != d->nextPosition;
            popup->move(popup->pos().x(), y);
        } else {
            d->nextPosition += popup->height();
        }
    }

    if (!cont) {
        killTimer(d->animationTimer);
        d->animationTimer = 0;
    }
}

void NotifyByPopup::onPassivePopupLinkClicked(const QString &link)
{
    unsigned int id = link.section(QLatin1Char('/') , 0 , 0).toUInt();
    unsigned int action = link.section(QLatin1Char('/') , 1 , 1).toUInt();

    if (id == 0 || action == 0) {
        return;
    }

    emit actionInvoked(id, action);
}

void NotifyByPopup::close(KNotification *notification)
{
    if (d->dbusServiceExists) {
        d->closeGalagoNotification(notification);
    }

    if (d->passivePopups.contains(notification)) {
        // this will call onPassivePopupDestroyed()
        // which will call finish() on the notification
        d->passivePopups[notification]->deleteLater();
    }

    QMutableListIterator<QPair<KNotification*, KNotifyConfig> > iter(d->notificationQueue);
    while (iter.hasNext()) {
        auto &item = iter.next();
        if (item.first == notification) {
            iter.remove();
        }
    }
}

void NotifyByPopup::update(KNotification *notification, KNotifyConfig *notifyConfig)
{
    update(notification, *notifyConfig);
}

void NotifyByPopup::update(KNotification *notification, const KNotifyConfig &notifyConfig)
{
    if (d->passivePopups.contains(notification)) {
        KPassivePopup *p = d->passivePopups[notification];
        d->fillPopup(p, notification, notifyConfig);
        return;
    }

    // if Notifications DBus service exists on bus,
    // it'll be used instead
    if (d->dbusServiceExists) {
        d->sendNotificationToGalagoServer(notification, notifyConfig, true);
        return;
    }
}

void NotifyByPopup::onServiceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(serviceName);
    // close all notifications we currently hold reference to
    for (KNotification *n : qAsConst(d->galagoNotifications)) {
        if (n) {
            emit finished(n);
        }
    }
    QMap<KNotification*, KPassivePopup *>::const_iterator i = d->passivePopups.constBegin();
    while (i != d->passivePopups.constEnd()) {
        emit finished(i.key());
        ++i;
    }
    d->galagoNotifications.clear();
    d->passivePopups.clear();

    d->dbusServiceCapCacheDirty = true;
    d->popupServerCapabilities.clear();

    if (newOwner.isEmpty()) {
        d->notificationQueue.clear();
        if (!d->dbusServiceActivatable) {
            d->dbusServiceExists = false;
        }
    } else if (oldOwner.isEmpty()) {
        d->dbusServiceExists = true;

        // connect to action invocation signals
        bool connected = QDBusConnection::sessionBus().connect(QString(), // from any service
                                                               QString::fromLatin1(dbusPath),
                                                               QString::fromLatin1(dbusInterfaceName),
                                                               QStringLiteral("ActionInvoked"),
                                                               this,
                                                               SLOT(onGalagoNotificationActionInvoked(uint,QString)));
        if (!connected) {
            qCWarning(LOG_KNOTIFICATIONS) << "warning: failed to connect to ActionInvoked dbus signal";
        }

        connected = QDBusConnection::sessionBus().connect(QString(), // from any service
                                                          QString::fromLatin1(dbusPath),
                                                          QString::fromLatin1(dbusInterfaceName),
                                                          QStringLiteral("NotificationClosed"),
                                                          this,
                                                          SLOT(onGalagoNotificationClosed(uint,uint)));
        if (!connected) {
            qCWarning(LOG_KNOTIFICATIONS) << "warning: failed to connect to NotificationClosed dbus signal";
        }
    }
}

void NotifyByPopup::onGalagoNotificationActionInvoked(uint notificationId, const QString &actionKey)
{
    auto iter = d->galagoNotifications.find(notificationId);
    if (iter == d->galagoNotifications.end()) {
        return;
    }

    KNotification *n = *iter;
    if (n) {
        if (actionKey == QLatin1String("default")) {
            emit actionInvoked(n->id(), 0);
        } else {
            emit actionInvoked(n->id(), actionKey.toUInt());
        }
    } else {
        d->galagoNotifications.erase(iter);
    }
}

void NotifyByPopup::onGalagoNotificationClosed(uint dbus_id, uint reason)
{
    auto iter = d->galagoNotifications.find(dbus_id);
    if (iter == d->galagoNotifications.end()) {
        return;
    }
    KNotification *n = *iter;
    d->galagoNotifications.remove(dbus_id);

    if (n) {
        emit finished(n);
        // The popup bubble is the only user facing part of a notification,
        // if the user closes the popup, it means he wants to get rid
        // of the notification completely, including playing sound etc
        // Therefore we close the KNotification completely after closing
        // the popup, but only if the reason is 2, which means "user closed"
        if (reason == 2) {
            n->close();
        }
    }
}

void NotifyByPopup::onGalagoServerReply(QDBusPendingCallWatcher *watcher)
{
    // call deleteLater first, since we might return in the middle of the function
    watcher->deleteLater();
    KNotification *notification = watcher->property("notificationObject").value<KNotification*>();
    if (!notification) {
        qCWarning(LOG_KNOTIFICATIONS) << "Invalid notification object passed in DBus reply watcher; notification will probably break";
        return;
    }

    QDBusPendingReply<uint> reply = *watcher;

    d->galagoNotifications.insert(reply.argumentAt<0>(), notification);
}

void NotifyByPopup::onGalagoServerCapabilitiesReceived(const QStringList &capabilities)
{
    d->popupServerCapabilities = capabilities;
    d->dbusServiceCapCacheDirty = false;

    // re-run notify() on all enqueued notifications
    for (int i = 0, total = d->notificationQueue.size(); i < total; ++i) {
        notify(d->notificationQueue.at(i).first, d->notificationQueue.at(i).second);
    }

    d->notificationQueue.clear();
}

void NotifyByPopupPrivate::getAppCaptionAndIconName(const KNotifyConfig &notifyConfig, QString *appCaption, QString *iconName)
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

void NotifyByPopupPrivate::fillPopup(KPassivePopup *popup, KNotification *notification, const KNotifyConfig &notifyConfig)
{
    QString appCaption;
    QString iconName;
    getAppCaptionAndIconName(notifyConfig, &appCaption, &iconName);

    // If we're at this place, it means there's no D-Bus service for notifications
    // so we don't need to do D-Bus query for the capabilities.
    // If queryPopupServerCapabilities() finds no service, it sets the KPassivePopup
    // capabilities immediately, so we don't need to wait for callback as in the case
    // of galago notifications
    queryPopupServerCapabilities();

    int iconDimension = QFontMetrics(QFont()).height();
    QPixmap appIcon = QIcon::fromTheme(iconName).pixmap(iconDimension, iconDimension);

    QWidget *vb = popup->standardView(notification->title().isEmpty() ? appCaption : notification->title(),
                                      notification->pixmap().isNull() ? notification->text() : QString(),
                                      appIcon);

    if (!notification->pixmap().isNull()) {
        const QPixmap pix = notification->pixmap();
        QHBoxLayout *hbox = new QHBoxLayout(vb);

        QLabel *pil = new QLabel();
        pil->setPixmap(pix);
        pil->setScaledContents(true);

        if (pix.height() > 80 && pix.height() > pix.width()) {
            pil->setMaximumHeight(80);
            pil->setMaximumWidth(80 * pix.width() / pix.height());
        } else if(pix.width() > 80 && pix.height() <= pix.width()) {
            pil->setMaximumWidth(80);
            pil->setMaximumHeight(80*pix.height()/pix.width());
        }

        hbox->addWidget(pil);

        QVBoxLayout *vb2 = new QVBoxLayout(vb);
        QLabel *msg = new QLabel(notification->text());
        msg->setAlignment(Qt::AlignLeft);

        vb2->addWidget(msg);

        hbox->addLayout(vb2);

        vb->layout()->addItem(hbox);
    }


    if (!notification->actions().isEmpty()) {
        QString linkCode = QStringLiteral("<p align=\"right\">");
        int i = 0;
        const auto actionList = notification->actions();
        for (const QString &it : actionList) {
            i++;
            linkCode += QStringLiteral("&nbsp;<a href=\"%1/%2\">%3</a>").arg(QString::number(notification->id()), QString::number(i), it.toHtmlEscaped());
        }

        linkCode += QLatin1String("</p>");
        QLabel *link = new QLabel(linkCode , vb );
        link->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        link->setOpenExternalLinks(false);
        //link->setAlignment( AlignRight );
        QObject::connect(link, &QLabel::linkActivated,
                         q, &NotifyByPopup::onPassivePopupLinkClicked);
        QObject::connect(link, &QLabel::linkActivated,
                         popup, &QWidget::hide);
    }

    popup->setView( vb );
}

bool NotifyByPopupPrivate::sendNotificationToGalagoServer(KNotification *notification, const KNotifyConfig &notifyConfig_nocheck, bool update)
{
    uint updateId = galagoNotifications.key(notification, 0);

    if (update) {
        if (updateId == 0) {
            // we have nothing to update; the notification we're trying to update
            // has been already closed
            return false;
        }
    }

    QDBusMessage dbusNotificationMessage = QDBusMessage::createMethodCall(QString::fromLatin1(dbusServiceName), QString::fromLatin1(dbusPath), QString::fromLatin1(dbusInterfaceName), QStringLiteral("Notify"));

    QList<QVariant> args;

    QString appCaption;
    QString iconName;
    getAppCaptionAndIconName(notifyConfig_nocheck, &appCaption, &iconName);

    //did the user override the icon name?
    if (!notification->iconName().isEmpty()) {
        iconName = notification->iconName();
    }

    args.append(appCaption); // app_name
    args.append(updateId);  // notification to update
    args.append(iconName); // app_icon

    QString title = notification->title().isEmpty() ? appCaption : notification->title();
    QString text = notification->text();

    if (!popupServerCapabilities.contains(QLatin1String("body-markup"))) {
        if (title.startsWith(QLatin1String("<html>"))) {
            title = q->stripRichText(title);
        }
        if (text.startsWith(QLatin1String("<html>"))) {
            text = q->stripRichText(text);
        }
    }

    args.append(title); // summary
    args.append(text); // body

    // galago spec defines action list to be list like
    // (act_id1, action1, act_id2, action2, ...)
    //
    // assign id's to actions like it's done in fillPopup() method
    // (i.e. starting from 1)
    QStringList actionList;
    if (popupServerCapabilities.contains(QLatin1String("actions"))) {
        QString defaultAction = notification->defaultAction();
        if (!defaultAction.isEmpty()) {
            actionList.append(QStringLiteral("default"));
            actionList.append(defaultAction);
        }
        int actId = 0;
        const auto listActions = notification->actions();
        for (const QString &actionName : listActions) {
            actId++;
            actionList.append(QString::number(actId));
            actionList.append(actionName);
        }
    }

    args.append(actionList); // actions

    QVariantMap hintsMap;
    // Add the application name to the hints.
    // According to fdo spec, the app_name is supposed to be the applicaton's "pretty name"
    // but in some places it's handy to know the application name itself
    if (!notification->appName().isEmpty()) {
        hintsMap[QStringLiteral("x-kde-appname")] = notification->appName();
    }

    if (!notification->eventId().isEmpty()) {
        hintsMap[QStringLiteral("x-kde-eventId")] = notification->eventId();
    }

    if (notification->flags() & KNotification::SkipGrouping) {
        hintsMap[QStringLiteral("x-kde-skipGrouping")] = 1;
    }

    if (!notification->urls().isEmpty()) {
        hintsMap[QStringLiteral("x-kde-urls")] = QUrl::toStringList(notification->urls());
    }

    if (!(notification->flags() & KNotification::Persistent)) {
        hintsMap[QStringLiteral("transient")] = true;
    }

    QString desktopFileName = QGuiApplication::desktopFileName();
    if (!desktopFileName.isEmpty()) {
        // handle apps which set the desktopFileName property with filename suffix,
        // due to unclear API dox (https://bugreports.qt.io/browse/QTBUG-75521)
        if (desktopFileName.endsWith(QLatin1String(".desktop"))) {
            desktopFileName.chop(8);
        }
        hintsMap[QStringLiteral("desktop-entry")] = desktopFileName;
    }

    int urgency = -1;
    switch (notification->urgency()) {
    case KNotification::DefaultUrgency:
        break;
    case KNotification::LowUrgency:
        urgency = 0;
        break;
    case KNotification::NormalUrgency:
        Q_FALLTHROUGH();
    // galago notifications only know low, normal, critical
    case KNotification::HighUrgency:
        urgency = 1;
        break;
    case KNotification::CriticalUrgency:
        urgency = 2;
        break;
    }

    if (urgency > -1) {
        hintsMap[QStringLiteral("urgency")] = urgency;
    }

    const QVariantMap hints = notification->hints();
    for (auto it = hints.constBegin(); it != hints.constEnd(); ++it) {
        hintsMap[it.key()] = it.value();
    }

    //FIXME - reenable/fix
    // let's see if we've got an image, and store the image in the hints map
    if (!notification->pixmap().isNull()) {
        QByteArray pixmapData;
        QBuffer buffer(&pixmapData);
        buffer.open(QIODevice::WriteOnly);
        notification->pixmap().save(&buffer, "PNG");
        buffer.close();
        hintsMap[QStringLiteral("image_data")] = ImageConverter::variantForImage(QImage::fromData(pixmapData));
    }

    args.append(hintsMap); // hints

    // Persistent     => 0  == infinite timeout
    // CloseOnTimeout => -1 == let the server decide
    int timeout = (notification->flags() & KNotification::Persistent) ? 0 : -1;

    args.append(timeout); // expire timout

    dbusNotificationMessage.setArguments(args);

    QDBusPendingCall notificationCall = QDBusConnection::sessionBus().asyncCall(dbusNotificationMessage, -1);

    //parent is set to the notification so that no-one ever accesses a dangling pointer on the notificationObject property
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(notificationCall, notification);
    watcher->setProperty("notificationObject", QVariant::fromValue<KNotification*>(notification));

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished,
                     q, &NotifyByPopup::onGalagoServerReply);

    return true;
}

void NotifyByPopupPrivate::closeGalagoNotification(KNotification *notification)
{
    uint galagoId = galagoNotifications.key(notification, 0);

    if (galagoId == 0) {
        qCDebug(LOG_KNOTIFICATIONS) << "not found dbus id to close" << notification->id();
        return;
    }

    QDBusMessage m = QDBusMessage::createMethodCall(QString::fromLatin1(dbusServiceName), QString::fromLatin1(dbusPath),
                                                    QString::fromLatin1(dbusInterfaceName), QStringLiteral("CloseNotification"));
    QList<QVariant> args;
    args.append(galagoId);
    m.setArguments(args);

    // send(..) does not block
    bool queued = QDBusConnection::sessionBus().send(m);

    if (!queued) {
        qCWarning(LOG_KNOTIFICATIONS) << "Failed to queue dbus message for closing a notification";
    }
}

void NotifyByPopupPrivate::queryPopupServerCapabilities()
{
    if (!dbusServiceExists) {
        // Return capabilities of the KPassivePopup implementation
        popupServerCapabilities = QStringList() << QStringLiteral("actions") << QStringLiteral("body") << QStringLiteral("body-hyperlinks")
                                                      << QStringLiteral("body-markup") << QStringLiteral("icon-static");
    }

    if (dbusServiceCapCacheDirty) {
        QDBusMessage m = QDBusMessage::createMethodCall(QString::fromLatin1(dbusServiceName),
                                                        QString::fromLatin1(dbusPath),
                                                        QString::fromLatin1(dbusInterfaceName),
                                                        QStringLiteral("GetCapabilities"));

        QDBusConnection::sessionBus().callWithCallback(m,
                                                       q,
                                                       SLOT(onGalagoServerCapabilitiesReceived(QStringList)),
                                                       nullptr,
                                                       -1);
    }
}



