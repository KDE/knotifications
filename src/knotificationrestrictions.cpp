/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knotificationrestrictions.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

#include "debug_p.h"
#include <config-knotifications.h>

#if HAVE_XTEST
#include <QTimer>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QX11Info>
#endif

#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#endif // HAVE_XTEST

class Q_DECL_HIDDEN KNotificationRestrictions::Private
{
public:
    Private(KNotificationRestrictions *qq, Services c, const QString &r)
        : q(qq)
        , control(c)
        , screenSaverDbusCookie(-1)
        , reason(r)
#if HAVE_XTEST
        , screensaverTimer(nullptr)
        , haveXTest(0)
        , XTestKeyCode(0)
        , isX11(QGuiApplication::platformName() == QLatin1String("xcb"))
#endif // HAVE_XTEST
    {
    }

    void screensaverFakeKeyEvent();
    void startScreenSaverPrevention();
    void stopScreenSaverPrevention();

    static QString determineProgramName();

    KNotificationRestrictions *q;
    Services control;
    int screenSaverDbusCookie;
    QString reason;
#if HAVE_XTEST
    QTimer *screensaverTimer;
    int haveXTest;
    int XTestKeyCode;
    bool isX11;
#endif // HAVE_XTEST
};

KNotificationRestrictions::KNotificationRestrictions(Services control, QObject *parent)
    : KNotificationRestrictions(control, QStringLiteral("no_reason_specified"), parent)
{
}

KNotificationRestrictions::KNotificationRestrictions(Services control, const QString &reason, QObject *parent)
    : QObject(parent)
    , d(new Private(this, control, reason))
{
    if (d->control & ScreenSaver) {
        d->startScreenSaverPrevention();
    }
}

KNotificationRestrictions::~KNotificationRestrictions()
{
    if (d->control & ScreenSaver) {
        d->stopScreenSaverPrevention();
    }
}

void KNotificationRestrictions::Private::screensaverFakeKeyEvent()
{
    qCDebug(LOG_KNOTIFICATIONS);
#if HAVE_XTEST
    if (!isX11) {
        return;
    }
    qCDebug(LOG_KNOTIFICATIONS) << "---- using XTestFakeKeyEvent";
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Display *display = QX11Info::display();
#else
    Display *display = qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->display();
#endif

    XTestFakeKeyEvent(display, XTestKeyCode, true, CurrentTime);
    XTestFakeKeyEvent(display, XTestKeyCode, false, CurrentTime);
    XSync(display, false);
#endif // HAVE_XTEST
}

void KNotificationRestrictions::Private::startScreenSaverPrevention()
{
    qCDebug(LOG_KNOTIFICATIONS);

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"),
                                                          QStringLiteral("/ScreenSaver"),
                                                          QStringLiteral("org.freedesktop.ScreenSaver"),
                                                          QStringLiteral("Inhibit"));
    message << determineProgramName();
    message << reason;
    QDBusReply<uint> reply = QDBusConnection::sessionBus().call(message);
    if (reply.isValid()) {
        screenSaverDbusCookie = reply.value();
        return;
    }
#if HAVE_XTEST
    if (!isX11) {
        return;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Display *display = QX11Info::display();
#else
    Display *display = qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->display();
#endif

    if (!haveXTest) {
        int a;
        int b;
        int c;
        int e;
        haveXTest = XTestQueryExtension(display, &a, &b, &c, &e);

        if (!haveXTest) {
            qCDebug(LOG_KNOTIFICATIONS) << "--- No XTEST!";
            return;
        }
    }

    if (!XTestKeyCode) {
        XTestKeyCode = XKeysymToKeycode(display, XK_Shift_L);

        if (!XTestKeyCode) {
            qCDebug(LOG_KNOTIFICATIONS) << "--- No XKeyCode for XK_Shift_L!";
            return;
        }
    }

    if (!screensaverTimer) {
        screensaverTimer = new QTimer(q);
        connect(screensaverTimer, SIGNAL(timeout()), q, SLOT(screensaverFakeKeyEvent()));
    }

    qCDebug(LOG_KNOTIFICATIONS) << "---- using XTest";
    // send a fake event right away in case this got started after a period of
    // inactivity leading to the screensaver set to activate in <55s
    screensaverFakeKeyEvent();
    screensaverTimer->start(55000);
#endif // HAVE_XTEST
}

void KNotificationRestrictions::Private::stopScreenSaverPrevention()
{
    if (screenSaverDbusCookie != -1) {
        QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"),
                                                              QStringLiteral("/ScreenSaver"),
                                                              QStringLiteral("org.freedesktop.ScreenSaver"),
                                                              QStringLiteral("UnInhibit"));
        message << static_cast<uint>(screenSaverDbusCookie);
        screenSaverDbusCookie = -1;
        if (QDBusConnection::sessionBus().send(message)) {
            return;
        }
    }
#if HAVE_XTEST
    if (!isX11) {
        return;
    }
    delete screensaverTimer;
    screensaverTimer = nullptr;
#endif // HAVE_XTEST
}

QString KNotificationRestrictions::Private::determineProgramName()
{
    QString appName = QGuiApplication::applicationDisplayName();
    if (appName.isEmpty()) {
        appName = QCoreApplication::applicationName();
    }
    if (appName.isEmpty()) {
        appName = tr("Unknown Application");
    }
    return appName;
}

#include "moc_knotificationrestrictions.cpp"
