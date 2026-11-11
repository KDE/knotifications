/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2006 Thiago Macieira <thiago@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNOTIFICATIONMANAGER_H
#define KNOTIFICATIONMANAGER_H

#include <knotification.h>

#include <memory>

class KNotification;
class QPixmap;
class KNotificationPlugin;

class KNotificationManager : public QObject
{
    Q_OBJECT
public:
    static KNotificationManager *self();
    ~KNotificationManager() override;

    void notify(KNotification *n);

    void close(int id);

    void update(KNotification *n);

private Q_SLOTS:
    void notificationClosed();
    void xdgActivationTokenReceived(int id, const QString &token);
    void notificationActivated(int id, const QString &action);
    void notificationReplied(int id, const QString &text);
    void notifyPluginFinished(KNotification *notification);
    void reparseConfiguration(const QString &app);

private:
    bool isInsideSandbox();

    KNotificationManager();

    friend class KNotificationManagerSingleton;

    QHash<int, KNotification *> notifications;
    KNotificationPlugin *plugin = nullptr;

    QStringList dirtyConfigCache;
    bool portalDBusServiceExists = false;
};

#endif
