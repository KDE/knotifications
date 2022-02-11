/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef NOTIFYBYANDROID_H
#define NOTIFYBYANDROID_H

#include "knotificationplugin.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAndroidJniObject>
#else
#include <QJniObject>
// TODO KF6 remove this porting aid
using QAndroidJniObject = QJniObject;
#endif
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
    void update(KNotification *notification, KNotifyConfig *config) override;
    void close(KNotification *notification) override;

    // interface from Java
    void notificationFinished(int id);
    void notificationActionInvoked(int id, int action);
    void notificationInlineReply(int id, const QString &text);

private:
    void notifyDeferred(KNotification *notification);
    QAndroidJniObject createAndroidNotification(KNotification *notification, KNotifyConfig *config) const;

    QAndroidJniObject m_backend;
    QHash<int, QPointer<KNotification>> m_notifications;
};

#endif // NOTIFYBYANDROID_H
