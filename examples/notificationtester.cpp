/*
   SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>

   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include <QDebug>
#include <QObject>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <KNotification>
#include <KNotificationReplyAction>

static QString defaultComponentName()
{
#if defined(Q_OS_ANDROID)
    return QStringLiteral("android_defaults");
#else
    return QStringLiteral("plasma_workspace");
#endif
}

class NotificationTester : public QObject
{
    Q_OBJECT

public:
    explicit NotificationTester(QObject *parent = nullptr) {}

    Q_INVOKABLE void sendNotification(const QString &title, const QString &text)
    {
        KNotification *notification = new KNotification(QStringLiteral("notification"));
        notification->setComponentName(defaultComponentName());
        notification->setTitle(title);
        notification->setText(text);

        notification->sendEvent();
    }

    Q_INVOKABLE void sendInlineReplyNotification(const QString &title, const QString &text)
    {
        auto notification = new KNotification(QStringLiteral("notification"));
        notification->setComponentName(defaultComponentName());
        notification->setTitle(title);
        notification->setText(text);
        std::unique_ptr<KNotificationReplyAction> replyAction(new KNotificationReplyAction(QStringLiteral("Reply")));
        replyAction->setPlaceholderText(QStringLiteral("Reply to annoying chat group..."));
        QObject::connect(replyAction.get(), &KNotificationReplyAction::replied, [](const QString &text) {
            qInfo() << "received reply:" << text;
        });
        notification->setReplyAction(std::move(replyAction));
        notification->sendEvent();
    }
};

#include "notificationtester.moc"

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif
int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    qmlRegisterSingletonType<NotificationTester>("org.kde.knotifications.tester", 1, 0, "Tester", [](QQmlEngine*, QJSEngine*) {
       return new NotificationTester;
    });

    engine.load(QStringLiteral("qrc:/notificationtester.qml"));

    return app.exec();
}
