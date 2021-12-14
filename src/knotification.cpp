/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2013-2014 Martin Klapetek <mklapetek@kde.org>

    code from KNotify/KNotifyClient
    SPDX-FileCopyrightText: 1997 Christian Esken <esken@kde.org>
    SPDX-FileCopyrightText: 2000 Charles Samuels <charles@kde.org>
    SPDX-FileCopyrightText: 2000 Stefan Schimanski <1Stein@gmx.de>
    SPDX-FileCopyrightText: 2000 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2000-2003 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2005 Allan Sandfeld Jensen <kde@carewolf.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "knotification.h"
#include "knotification_p.h"
#include "knotificationmanager_p.h"
#include "knotificationreplyaction.h"

#include <config-knotifications.h>

#if HAVE_KWINDOWSYSTEM
#include <KWindowSystem>
#endif
#include <QGuiApplication>

#include <QStringList>
#ifdef QT_WIDGETS_LIB
#include <QTabWidget>
#endif
#include <QUrl>

// incremental notification ID
static int notificationIdCounter = 0;


#if KNOTIFICATIONS_BUILD_DEPRECATED_SINCE(5, 75)
KNotification::KNotification(const QString &eventId, QWidget *parent, const NotificationFlags &flags)
    : QObject(parent)
    , d(new Private)
{
    d->eventId = eventId;
    d->flags = flags;
    setWidget(parent);
    connect(&d->updateTimer, &QTimer::timeout, this, &KNotification::update);
    d->updateTimer.setSingleShot(true);
    d->updateTimer.setInterval(100);
    d->id = ++notificationIdCounter;

#if HAVE_KWINDOWSYSTEM
    if (KWindowSystem::isPlatformWayland()) {
        setHint(QStringLiteral("x-kde-xdgTokenAppId"), QGuiApplication::desktopFileName());
    }
#endif
}
#endif

KNotification::KNotification(const QString &eventId, const NotificationFlags &flags, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->eventId = eventId;
    d->flags = flags;
    connect(&d->updateTimer, &QTimer::timeout, this, &KNotification::update);
    d->updateTimer.setSingleShot(true);
    d->updateTimer.setInterval(100);
    d->id = ++notificationIdCounter;

#if HAVE_KWINDOWSYSTEM
    if (KWindowSystem::isPlatformWayland()) {
        setHint(QStringLiteral("x-kde-xdgTokenAppId"), QGuiApplication::desktopFileName());
    }
#endif
}

KNotification::~KNotification()
{
    if (d->id >= 0) {
        KNotificationManager::self()->close(d->id);
    }
    delete d;
}

QString KNotification::eventId() const
{
    return d->eventId;
}

void KNotification::setEventId(const QString &eventId)
{
    if (d->eventId != eventId) {
        d->eventId = eventId;
        Q_EMIT eventIdChanged();
    }
}

QString KNotification::title() const
{
    return d->title;
}

QString KNotification::text() const
{
    return d->text;
}

QWidget *KNotification::widget() const
{
    return d->widget;
}

void KNotification::setWidget(QWidget *wid)
{
#ifdef QT_WIDGETS_LIB
    d->widget = wid;
    //     setParent(wid);
    if (wid && d->flags & CloseWhenWidgetActivated) {
        wid->installEventFilter(this);
    }
#endif
}

void KNotification::setTitle(const QString &title)
{
    if (title == d->title) {
        return;
    }

    d->needUpdate = true;
    d->title = title;
    Q_EMIT titleChanged();
    if (d->id >= 0) {
        d->updateTimer.start();
    }
}

void KNotification::setText(const QString &text)
{
    if (text == d->text) {
        return;
    }

    d->needUpdate = true;
    d->text = text;
    Q_EMIT textChanged();
    if (d->id >= 0) {
        d->updateTimer.start();
    }
}

void KNotification::setIconName(const QString &icon)
{
    if (icon == d->iconName) {
        return;
    }

    d->needUpdate = true;
    d->iconName = icon;
    Q_EMIT iconNameChanged();
    if (d->id >= 0) {
        d->updateTimer.start();
    }
}

QString KNotification::iconName() const
{
    return d->iconName;
}

QPixmap KNotification::pixmap() const
{
    return d->pixmap;
}

void KNotification::setPixmap(const QPixmap &pix)
{
    d->needUpdate = true;
    d->pixmap = pix;
    if (d->id >= 0) {
        d->updateTimer.start();
    }
}

QStringList KNotification::actions() const
{
    return d->actions;
}

void KNotification::setActions(const QStringList &as)
{
    if (as == d->actions) {
        return;
    }

    d->needUpdate = true;
    d->actions = as;
    Q_EMIT actionsChanged();
    if (d->id >= 0) {
        d->updateTimer.start();
    }
}

KNotificationReplyAction *KNotification::replyAction() const
{
    return d->replyAction.get();
}

void KNotification::setReplyAction(std::unique_ptr<KNotificationReplyAction> replyAction)
{
    if (replyAction == d->replyAction) {
        return;
    }

    d->needUpdate = true;
    d->replyAction = std::move(replyAction);
    if (d->id >= 0) {
        d->updateTimer.start();
    }
}

void KNotification::setDefaultAction(const QString &defaultAction)
{
    if (defaultAction == d->defaultAction) {
        return;
    }

    d->needUpdate = true;
    d->defaultAction = defaultAction;
    Q_EMIT defaultActionChanged();
    if (d->id >= 0) {
        d->updateTimer.start();
    }
}

QString KNotification::defaultAction() const
{
    return d->defaultAction;
}

KNotification::ContextList KNotification::contexts() const
{
    return d->contexts;
}

void KNotification::setContexts(const KNotification::ContextList &contexts)
{
    d->contexts = contexts;
}

void KNotification::addContext(const KNotification::Context &context)
{
    d->contexts << context;
}

void KNotification::addContext(const QString &context_key, const QString &context_value)
{
    d->contexts << qMakePair(context_key, context_value);
}

KNotification::NotificationFlags KNotification::flags() const
{
    return d->flags;
}

void KNotification::setFlags(const NotificationFlags &flags)
{
    if (d->flags == flags) {
        return;
    }

    d->needUpdate = true;
    d->flags = flags;
    Q_EMIT flagsChanged();
    if (d->id >= 0) {
        d->updateTimer.start();
    }
}

QString KNotification::componentName() const
{
    return d->componentName;
}

void KNotification::setComponentName(const QString &c)
{
    if (d->componentName != c) {
        d->componentName = c;
        Q_EMIT componentNameChanged();
    }
}

QList<QUrl> KNotification::urls() const
{
    return QUrl::fromStringList(d->hints[QStringLiteral("x-kde-urls")].toStringList());
}

void KNotification::setUrls(const QList<QUrl> &urls)
{
    setHint(QStringLiteral("x-kde-urls"), QUrl::toStringList(urls));
    Q_EMIT urlsChanged();
}

KNotification::Urgency KNotification::urgency() const
{
    return d->urgency;
}

void KNotification::setUrgency(Urgency urgency)
{
    if (d->urgency == urgency) {
        return;
    }

    d->needUpdate = true;
    d->urgency = urgency;
    Q_EMIT urgencyChanged();
    if (d->id >= 0) {
        d->updateTimer.start();
    }
}

void KNotification::activate(unsigned int action)
{
    switch (action) {
    case 0:
#if KNOTIFICATIONS_BUILD_DEPRECATED_SINCE(5, 76)
        Q_EMIT activated();
#endif
        Q_EMIT defaultActivated();
        break;
    case 1:
        Q_EMIT action1Activated();
        break;
    case 2:
        Q_EMIT action2Activated();
        break;
    case 3:
        Q_EMIT action3Activated();
        break;
    }

    // emitting activated() makes the Manager close all the active plugins
    // which will deref() the KNotification object, which will result
    // in closing the notification
    Q_EMIT activated(action);
}

void KNotification::close()
{
    if (d->id >= 0) {
        KNotificationManager::self()->close(d->id);
    }

    if (d->id == -1) {
        d->id = -2;
        Q_EMIT closed();
        if (d->autoDelete) {
            deleteLater();
        } else {
            // reset for being reused
            d->isNew = true;
            d->id = ++notificationIdCounter;
        }
    }
}

#if KNOTIFICATIONS_BUILD_DEPRECATED_SINCE(5, 67)
void KNotification::raiseWidget()
{
    if (!d->widget) {
        return;
    }

    d->Private::raiseWidget(d->widget);
}
#endif

#if KNOTIFICATIONS_BUILD_DEPRECATED_SINCE(5, 67)
void KNotification::Private::raiseWidget(QWidget *w)
{
    // TODO  this function is far from finished.
    if (w->isTopLevel()) {
        w->raise();
#if HAVE_KWINDOWSYSTEM
        if (!xdgActivationToken.isEmpty()) {
            KWindowSystem::setCurrentXdgActivationToken(xdgActivationToken);
        }
        KWindowSystem::activateWindow(w->winId());
#endif
    } else {
        QWidget *pw = w->parentWidget();
        raiseWidget(pw);

        if (QTabWidget *tab_widget = qobject_cast<QTabWidget *>(pw)) {
            tab_widget->setCurrentIndex(tab_widget->indexOf(w));
        }
    }
}
#endif

static QString defaultComponentName()
{
#if defined(Q_OS_ANDROID)
    return QStringLiteral("android_defaults");
#else
    return QStringLiteral("plasma_workspace");
#endif
}

KNotification *KNotification::event(const QString &eventid,
                                    const QString &title,
                                    const QString &text,
                                    const QPixmap &pixmap,
                                    QWidget *widget,
                                    const NotificationFlags &flags,
                                    const QString &componentName)
{
    KNotification *notify = new KNotification(eventid, flags);
    notify->setWidget(widget);
    notify->setTitle(title);
    notify->setText(text);
    notify->setPixmap(pixmap);
    notify->setComponentName((flags & DefaultEvent) ? defaultComponentName() : componentName);

    QTimer::singleShot(0, notify, &KNotification::sendEvent);

    return notify;
}

KNotification *KNotification::event(const QString &eventid,
                                    const QString &text,
                                    const QPixmap &pixmap,
                                    QWidget *widget,
                                    const NotificationFlags &flags,
                                    const QString &componentName)
{
    return event(eventid, QString(), text, pixmap, widget, flags, componentName);
}

KNotification *
KNotification::event(StandardEvent eventid, const QString &title, const QString &text, const QPixmap &pixmap, QWidget *widget, const NotificationFlags &flags)
{
    return event(standardEventToEventId(eventid), title, text, pixmap, widget, flags | DefaultEvent);
}

KNotification *KNotification::event(StandardEvent eventid, const QString &text, const QPixmap &pixmap, QWidget *widget, const NotificationFlags &flags)
{
    return event(eventid, QString(), text, pixmap, widget, flags);
}

KNotification *KNotification::event(const QString &eventid,
                                    const QString &title,
                                    const QString &text,
                                    const QString &iconName,
                                    QWidget *widget,
                                    const NotificationFlags &flags,
                                    const QString &componentName)
{
    KNotification *notify = new KNotification(eventid, flags);
    notify->setWidget(widget);
    notify->setTitle(title);
    notify->setText(text);
    notify->setIconName(iconName);
    notify->setComponentName((flags & DefaultEvent) ? defaultComponentName() : componentName);

    QTimer::singleShot(0, notify, &KNotification::sendEvent);

    return notify;
}

KNotification *
KNotification::event(StandardEvent eventid, const QString &title, const QString &text, const QString &iconName, QWidget *widget, const NotificationFlags &flags)
{
    return event(standardEventToEventId(eventid), title, text, iconName, widget, flags | DefaultEvent);
}

KNotification *KNotification::event(StandardEvent eventid, const QString &title, const QString &text, QWidget *widget, const NotificationFlags &flags)
{
    return event(standardEventToEventId(eventid), title, text, standardEventToIconName(eventid), widget, flags | DefaultEvent);
}

void KNotification::ref()
{
    d->ref++;
}
void KNotification::deref()
{
    Q_ASSERT(d->ref > 0);
    d->ref--;
    if (d->ref == 0) {
        d->id = -1;
        close();
    }
}

void KNotification::beep(const QString &reason, QWidget *widget)
{
    event(QStringLiteral("beep"), reason, QPixmap(), widget, CloseOnTimeout | DefaultEvent);
}

void KNotification::sendEvent()
{
    d->needUpdate = false;
    if (d->isNew) {
        d->isNew = false;
        KNotificationManager::self()->notify(this);
    } else {
        KNotificationManager::self()->reemit(this);
    }
}

int KNotification::id()
{
    if (!d) {
        return -1;
    }
    return d->id;
}

QString KNotification::appName() const
{
    QString appname;

    if (d->flags & DefaultEvent) {
        appname = defaultComponentName();
    } else if (!d->componentName.isEmpty()) {
        appname = d->componentName;
    } else {
        appname = QCoreApplication::applicationName();
    }

    return appname;
}

bool KNotification::isAutoDelete() const
{
    return d->autoDelete;
}

void KNotification::setAutoDelete(bool autoDelete)
{
    if (d->autoDelete != autoDelete) {
        d->autoDelete = autoDelete;
        Q_EMIT autoDeleteChanged();
    }
}

void KNotification::update()
{
    if (d->needUpdate) {
        KNotificationManager::self()->update(this);
    }
}

bool KNotification::eventFilter(QObject *watched, QEvent *event)
{
#ifdef QT_WIDGETS_LIB
    if (watched == d->widget) {
        if (event->type() == QEvent::WindowActivate) {
            if (d->flags & CloseWhenWidgetActivated) {
                QTimer::singleShot(500, this, &KNotification::close);
            }
        }
    }
#endif

    return false;
}

QString KNotification::standardEventToEventId(KNotification::StandardEvent event)
{
    QString eventId;
    switch (event) {
    case Warning:
        eventId = QStringLiteral("warning");
        break;
    case Error:
        eventId = QStringLiteral("fatalerror");
        break;
    case Catastrophe:
        eventId = QStringLiteral("catastrophe");
        break;
    case Notification: // fall through
    default:
        eventId = QStringLiteral("notification");
        break;
    }
    return eventId;
}

QString KNotification::standardEventToIconName(KNotification::StandardEvent event)
{
    QString iconName;
    switch (event) {
    case Warning:
        iconName = QStringLiteral("dialog-warning");
        break;
    case Error:
        iconName = QStringLiteral("dialog-error");
        break;
    case Catastrophe:
        iconName = QStringLiteral("dialog-error");
        break;
    case Notification: // fall through
    default:
        iconName = QStringLiteral("dialog-information");
        break;
    }
    return iconName;
}

void KNotification::setHint(const QString &hint, const QVariant &value)
{
    if (value == d->hints.value(hint)) {
        return;
    }

    d->needUpdate = true;
    d->hints[hint] = value;
    if (d->id >= 0) {
        d->updateTimer.start();
    }
}

QVariantMap KNotification::hints() const
{
    return d->hints;
}

QString KNotification::xdgActivationToken() const
{
    return d->xdgActivationToken;
}
