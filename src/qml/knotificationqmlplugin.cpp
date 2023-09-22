/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knotificationqmlplugin.h"

#include <KNotification>
#include <KNotificationPermission>
#include <KNotificationReplyAction>

class NotificationWrapper : public KNotification
{
    Q_OBJECT
    Q_PROPERTY(KNotificationReplyAction *replyAction READ replyActionFactory CONSTANT)
public:
    explicit NotificationWrapper(QObject *parent = nullptr)
        : KNotification(QString(), KNotification::CloseOnTimeout, parent)
    {
        setAutoDelete(false);
    }

    KNotificationReplyAction *replyActionFactory()
    {
        if (!replyAction()) {
            setReplyAction(std::make_unique<KNotificationReplyAction>(QString()));
        }
        return replyAction();
    }
};

class NotificationPermissionWrapper
{
    Q_GADGET
public:
    Q_INVOKABLE bool checkPermission()
    {
        return KNotificationPermission::checkPermission() == Qt::PermissionStatus::Granted;
    }

    Q_INVOKABLE void requestPermission(const QJSValue &callback)
    {
        KNotificationPermission::requestPermission(m_engine, [&callback](Qt::PermissionStatus status) {
            callback.call({status == Qt::PermissionStatus::Granted});
        });
    }

    QQmlEngine *m_engine = nullptr;
};

void KNotificationQmlPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.notification"));
    qmlRegisterType<NotificationWrapper>(uri, 1, 0, "Notification");
    qmlRegisterUncreatableType<KNotificationReplyAction>(uri, 1, 0, "NotificationReplyAction", {});
    qmlRegisterSingletonType("org.kde.notification", 1, 0, "NotificationPermission", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
        return engine->toScriptValue(NotificationPermissionWrapper{engine});
    });
}

#include "knotificationqmlplugin.moc"
#include "moc_knotificationqmlplugin.cpp"
