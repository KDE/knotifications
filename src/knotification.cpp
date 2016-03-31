/* This file is part of the KDE libraries
   Copyright (C) 2005-2006 Olivier Goffart <ogoffart at kde.org>
   Copyright (C) 2013-2014 Martin Klapetek <mklapetek@kde.org>

   code from KNotify/KNotifyClient
   Copyright (c) 1997 Christian Esken (esken@kde.org)
                 2000 Charles Samuels (charles@kde.org)
                 2000 Stefan Schimanski (1Stein@gmx.de)
                 2000 Matthias Ettrich (ettrich@kde.org)
                 2000 Waldo Bastian <bastian@kde.org>
                 2000-2003 Carsten Pfeiffer <pfeiffer@kde.org>
                 2005 Allan Sandfeld Jensen <kde@carewolf.com>

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

#include "knotification.h"
#include "knotificationmanager_p.h"

#include <kwindowsystem.h>

#include <QCoreApplication>

#include <QMap>
#include <QPixmap>
#include <QPointer>
#include <QLabel>
#include <QTimer>
#include <QTabWidget>
#include <QFile>
#include <QStringList>

struct KNotification::Private {
    QString eventId;
    int id;
    int ref;

    QWidget *widget;
    QString title;
    QString text;
    QString iconName;
    QStringList actions;
    QPixmap pixmap;
    ContextList contexts;
    NotificationFlags flags;
    QString componentName;

    QTimer updateTimer;
    bool needUpdate;

    Private() : id(-1), ref(0), widget(0l), needUpdate(false) {}
    /**
     * recursive function that raise the widget. @p w
     *
     * @see raiseWidget()
     */
    static void raiseWidget(QWidget *w);
};

KNotification::KNotification(const QString &eventId, QWidget *parent, const NotificationFlags &flags) :
    QObject(parent), d(new Private)
{
    d->eventId = eventId;
    d->flags = flags;
    setWidget(parent);
    connect(&d->updateTimer, SIGNAL(timeout()), this, SLOT(update()));
    d->updateTimer.setSingleShot(true);
    d->updateTimer.setInterval(100);
}

KNotification::KNotification(
    const QString &eventId,
    const NotificationFlags &flags,
    QObject *parent)
    :   QObject(parent),
        d(new Private)
{
    d->eventId = eventId;
    d->flags = flags;
    connect(&d->updateTimer, SIGNAL(timeout()), this, SLOT(update()));
    d->updateTimer.setSingleShot(true);
    d->updateTimer.setInterval(100);
    d->widget = nullptr;
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
    d->widget = wid;
//     setParent(wid);
    if (wid && d->flags &  CloseWhenWidgetActivated) {
        wid->installEventFilter(this);
    }
}

void KNotification::setTitle(const QString &title)
{
    if (title == d->title) {
        return;
    }

    d->needUpdate = true;
    d->title = title;
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
    if (d->id >= 0) {
        d->updateTimer.start();
    }
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
    d->flags = flags;
}

void KNotification::setComponentName(const QString &c)
{
    d->componentName = c;
}

void KNotification::activate(unsigned int action)
{
    switch (action) {
    case 0:
        emit activated();
        break;
    case 1:
        emit action1Activated();
        break;
    case 2:
        emit action2Activated();
        break;
    case 3:
        emit action3Activated();
        break;
    }

    // emitting activated() makes the Manager close all the active plugins
    // which will deref() the KNotification object, which will result
    // in closing the notification
    emit activated(action);
}

void KNotification::close()
{
    if (d->id >= 0) {
        KNotificationManager::self()->close(d->id);
    }

    if (d->id == -1) {
        d->id = -2;
        emit closed();
        deleteLater();
    }
}

void KNotification::raiseWidget()
{
    if (!d->widget) {
        return;
    }

    Private::raiseWidget(d->widget);
}

void KNotification::Private::raiseWidget(QWidget *w)
{
    //TODO  this function is far from finished.
    if (w->isTopLevel()) {
        w->raise();
#if defined(Q_OS_MAC)
        w->activateWindow();
#else
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

KNotification *KNotification::event(const QString &eventid, const QString &title, const QString &text,
                                    const QPixmap &pixmap, QWidget *widget, const NotificationFlags &flags, const QString &componentName)
{
    KNotification *notify = new KNotification(eventid, widget, flags);
    notify->setTitle(title);
    notify->setText(text);
    notify->setPixmap(pixmap);
    notify->setComponentName(flags & DefaultEvent ? QStringLiteral("plasma_workspace") : componentName);

    QTimer::singleShot(0, notify, SLOT(sendEvent()));

    return notify;
}

KNotification *KNotification::event(const QString &eventid, const QString &text,
                                    const QPixmap &pixmap, QWidget *widget, const NotificationFlags &flags, const QString &componentName)
{
    return event(eventid, QString(), text, pixmap, widget, flags, componentName);
}

KNotification *KNotification::event(StandardEvent eventid, const QString &title, const QString &text,
                                    const QPixmap &pixmap, QWidget *widget, const NotificationFlags &flags)
{

    return event(standardEventToEventId(eventid), title, text, pixmap, widget, flags | DefaultEvent);
}

KNotification *KNotification::event(StandardEvent eventid, const QString &text,
                                    const QPixmap &pixmap, QWidget *widget, const NotificationFlags &flags)
{
    return event(eventid, QString(), text, pixmap, widget, flags);
}

KNotification *KNotification::event(const QString &eventid, const QString &title, const QString &text,
                                const QString &iconName, QWidget *widget,
                                const NotificationFlags &flags, const QString &componentName)
{
    KNotification *notify = new KNotification(eventid, widget, flags);
    notify->setTitle(title);
    notify->setText(text);
    notify->setIconName(iconName);
    notify->setComponentName(flags & DefaultEvent ? QStringLiteral("plasma_workspace") : componentName);

    QTimer::singleShot(0, notify, SLOT(sendEvent()));

    return notify;
}

KNotification *KNotification::event(StandardEvent eventid, const QString &title, const QString &text,
                                    const QString &iconName, QWidget *widget,
                                    const NotificationFlags &flags)
{
    return event(standardEventToEventId(eventid), title, text, iconName, widget, flags | DefaultEvent);
}

KNotification* KNotification::event(StandardEvent eventid, const QString &title, const QString &text,
                                    QWidget *widget, const NotificationFlags &flags)
{
    return event(standardEventToEventId(eventid), title, text, standardEventToIconName(eventid), widget, flags | DefaultEvent);
}

void KNotification::ref()
{
    d->ref++;
}
void KNotification::deref()
{
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
    if (d->id == -1) {
        d->id = KNotificationManager::self()->notify(this);
    } else if (d->id >= 0) {
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
        appname = QStringLiteral("plasma_workspace");
    } else if (!d->componentName.isEmpty()) {
        appname = d->componentName;
    } else {
        appname = QCoreApplication::applicationName();
    }

    return appname;
}

void KNotification::update()
{
    if (d->needUpdate) {
        KNotificationManager::self()->update(this);
    }
}

bool KNotification::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->widget) {
        if (event->type() == QEvent::WindowActivate) {
            if (d->flags &  CloseWhenWidgetActivated) {
                QTimer::singleShot(500, this, SLOT(close()));
            }
        }
    }

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

