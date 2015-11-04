/*
    Copyright 2014 Martin Klapetek <mklapetek@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <knotification.h>
#include <QTimer>

#include <QApplication>
#include <QLabel>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>

static const char dbusServiceName[] = "org.freedesktop.Notifications";
static const char dbusInterfaceName[] = "org.freedesktop.Notifications";
static const char dbusPath[] = "/org/freedesktop/Notifications";

void notificationDBusCall(const QString &iconName, const QString &title, const QString &body, const QStringList &actions, bool persistent = false)
{
    QDBusMessage dbusNotificationMessage = QDBusMessage::createMethodCall(dbusServiceName, dbusPath, dbusInterfaceName, QStringLiteral("Notify"));

    QList<QVariant> args;

    args.append(QString()); // app_name
    args.append((uint)0);  // notification to update
    args.append(iconName); // app_icon
    args.append(title); // summary
    args.append(body); // body

    QStringList actionList;
    int actId = 0;
    Q_FOREACH (const QString &actionName, actions) {
        actId++;
        actionList.append(QString::number(actId));
        actionList.append(actionName);
    }

    args.append(actionList); // actions

   args.append(QVariantMap()); // hints

    // Persistent     => 0  == infinite timeout
    // CloseOnTimeout => -1 == let the server decide
    int timeout = persistent ? 0 : -1;

    args.append(timeout); // expire timout

    dbusNotificationMessage.setArguments(args);

    QDBusMessage reply = QDBusConnection::sessionBus().call(dbusNotificationMessage, QDBus::Block, 4000);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qDebug() << "Error sending notification:" << reply.errorMessage();
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    notificationDBusCall(QStringLiteral("amarok"),
                         QStringLiteral("Testing notification #1"),
                         QStringLiteral("Lorem ipsum dolor sit amet, consectetur adipiscing elit. In condimentum"),
                         QStringList() << QStringLiteral("action1") << QStringLiteral("action2"));

    // wait a little before sending another notification
    QEventLoop a;
    QTimer::singleShot(500, &a, SLOT(quit()));
    a.exec();

    notificationDBusCall(QStringLiteral("kwalletmanager"),
                         QStringLiteral("Testing notification #2"),
                         QStringLiteral("Praesent odio ipsum, posuere a magna ac, egestas vehicula lectus"),
                         QStringList() << QStringLiteral("action1") << QStringLiteral("action2"));

    QTimer::singleShot(1000, &a, SLOT(quit()));
    a.exec();

    notificationDBusCall(QStringLiteral("preferences-desktop-accessibility"),
                         QStringLiteral("Testing notification #3"),
                         QStringLiteral("Fusce hendrerit egestas pellentesque"),
                         QStringList(),
                         true);

    QTimer::singleShot(2000, &app, SLOT(quit()));

    return app.exec();
}

