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
    Q_PROPERTY(QQmlListProperty<KNotificationAction> actions READ actionsProperty NOTIFY actionsChanged)
    Q_PROPERTY(KNotificationAction *defaultAction READ defaultAction WRITE setDefaultAction NOTIFY defaultActionChanged)
public:
    explicit NotificationWrapper(QObject *parent = nullptr)
        : KNotification(QString(), KNotification::CloseOnTimeout, parent)
    {
        setAutoDelete(false);

        m_actionsProperty = QQmlListProperty<KNotificationAction>(this,
                                                                  nullptr,
                                                                  &NotificationWrapper::appendAction,
                                                                  &NotificationWrapper::actionsCount,
                                                                  &NotificationWrapper::actionAt,
                                                                  &NotificationWrapper::clearActions);
    }

    KNotificationReplyAction *replyActionFactory()
    {
        if (!replyAction()) {
            setReplyAction(std::make_unique<KNotificationReplyAction>(QString()));
        }
        return replyAction();
    }

    int actionCount() const
    {
        return actions().count();
    }

    KNotificationAction *actionAt(qsizetype index)
    {
        return actions().at(index);
    }

    QQmlListProperty<KNotificationAction> actionsProperty() const
    {
        return m_actionsProperty;
    }

    static qsizetype actionsCount(QQmlListProperty<KNotificationAction> *list)
    {
        return static_cast<NotificationWrapper *>(list->object)->actionCount();
    }

    static void appendAction(QQmlListProperty<KNotificationAction> *list, KNotificationAction *value)
    {
        auto notification = static_cast<NotificationWrapper *>(list->object);
        auto actions = notification->actions();
        actions << value;
        notification->setActions(actions);
    }

    static KNotificationAction *actionAt(QQmlListProperty<KNotificationAction> *list, qsizetype index)
    {
        return static_cast<NotificationWrapper *>(list->object)->actionAt(index);
    }

    static void clearActions(QQmlListProperty<KNotificationAction> *list)
    {
        auto notification = static_cast<KNotification *>(list->object);
        notification->clearActions();
    }

private:
    QQmlListProperty<KNotificationAction> m_actionsProperty;
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
    qmlRegisterType<KNotificationAction>(uri, 1, 0, "NotificationAction");
    qmlRegisterUncreatableType<KNotificationReplyAction>(uri, 1, 0, "NotificationReplyAction", {});
    qmlRegisterSingletonType("org.kde.notification", 1, 0, "NotificationPermission", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
        return engine->toScriptValue(NotificationPermissionWrapper{engine});
    });
}

#include "knotificationqmlplugin.moc"
#include "moc_knotificationqmlplugin.cpp"
