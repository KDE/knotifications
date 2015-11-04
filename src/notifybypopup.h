/*
   Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>
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

#ifndef NOTIFYBYPOPUP_H
#define NOTIFYBYPOPUP_H

#include "knotificationplugin.h"

#include <QStringList>

class KNotification;
class KPassivePopup;
class QDBusPendingCallWatcher;
class NotifyByPopupPrivate;

class NotifyByPopup : public KNotificationPlugin
{
    Q_OBJECT
public:
    NotifyByPopup(QObject *parent = 0l);
    virtual ~NotifyByPopup();

    QString optionName() Q_DECL_OVERRIDE { return QStringLiteral("Popup"); }
    void notify(KNotification *notification, KNotifyConfig *notifyConfig) Q_DECL_OVERRIDE;
    void close(KNotification *notification) Q_DECL_OVERRIDE;
    void update(KNotification *notification, KNotifyConfig *config) Q_DECL_OVERRIDE;

protected:
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void onPassivePopupDestroyed();
    void onPassivePopupLinkClicked(const QString &link);
    // slot to catch appearance or dissapearance of Notifications DBus service
    void onServiceOwnerChanged(const QString &, const QString &, const QString &);
    // slot which gets called when DBus signals that some notification action was invoked
    void onGalagoNotificationActionInvoked(uint notificationId, const QString &actionKey);
    // slot which gets called when DBus signals that some notification was closed
    void onGalagoNotificationClosed(uint, uint);

    void onGalagoServerReply(QDBusPendingCallWatcher *callWatcher);

    void onGalagoServerCapabilitiesReceived(const QStringList &capabilities);

private:
    // TODO KF6, replace current public notify/update
    void notify(KNotification *notification, const KNotifyConfig &notifyConfig);
    void update(KNotification *notification, const KNotifyConfig &notifyConfig);

    NotifyByPopupPrivate * const d;
};

#endif
