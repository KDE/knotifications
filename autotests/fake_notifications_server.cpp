/*
   Copyright (C) 2016 Martin Klapetek <mklapetek@kde.org>

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

#include "fake_notifications_server.h"

NotificationsServer::NotificationsServer(QObject *parent)
{
    Q_UNUSED(parent);
    counter = 1;
}

uint NotificationsServer::Notify(const QString &app_name,
                                 uint replaces_id,
                                 const QString &app_icon,
                                 const QString &summary,
                                 const QString &body,
                                 const QStringList &actions,
                                 const QVariantMap &hints,
                                 int timeout)
{
    NotificationItem i;
    i.app_name = app_name;
    i.replaces_id = replaces_id;
    i.app_icon = app_icon;
    i.summary = summary;
    i.body = body;
    i.actions = actions;
    i.hints = hints;
    i.timeout = timeout;
    i.id = counter;

    notifications.append(i);

    Q_EMIT newNotification();

    return counter++;
}

void NotificationsServer::CloseNotification(uint id)
{
    QList<NotificationItem>::iterator i = notifications.begin();
    while (i != notifications.end()) {
        if ((*i).id == id) {
            notifications.erase(i);
            break;
        }
        i++;
    }

    Q_EMIT NotificationClosed(id, 3);
}

QStringList NotificationsServer::GetCapabilities()
{
    return QStringList{QStringLiteral("body-markup"), QStringLiteral("body"), QStringLiteral("actions")};
}

QString NotificationsServer::GetServerInformation(QString &vendor, QString &version, QString &specVersion)
{
    vendor = QLatin1String("KDE");
    version = QLatin1String("2.0"); // FIXME
    specVersion = QLatin1String("1.1");
    return QStringLiteral("TestServer");
}
