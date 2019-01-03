/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef NOTIFYBYANDROID_H
#define NOTIFYBYANDROID_H

#include "knotificationplugin.h"

#include <QAndroidJniObject>
#include <QPointer>

/** Android notification backend. */
class NotifyByAndroid : public KNotificationPlugin
{
    Q_OBJECT
public:
    explicit NotifyByAndroid(QObject *parent = nullptr);
    ~NotifyByAndroid() override;

    // interface of KNotificationPlugin
    QString optionName() override;
    void notify(KNotification *notification, KNotifyConfig *config) override;
    void close(KNotification * notification) override;

    // interface from Java
    void notificationFinished(int id);
    void notificationActionInvoked(int id, int action);

private:
    void notifyDeferred(KNotification *notification, const KNotifyConfig *config);

    QAndroidJniObject m_backend;
    QHash<int, QPointer<KNotification>> m_notifications;
};

#endif // NOTIFYBYANDROID_H
