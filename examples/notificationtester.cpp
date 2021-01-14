/*
   SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>

   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include <QObject>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <KNotification>

class NotificationTester : public QObject
{
    Q_OBJECT

public:
    explicit NotificationTester(QObject *parent = nullptr) {}

    Q_INVOKABLE void sendNotification(const QString &title, const QString &text)
    {
        KNotification *notification = new KNotification(QStringLiteral("notification"));
#ifndef Q_OS_ANDROID
        notification->setComponentName(QStringLiteral("plasma_workspace"));
#else
        notification->setComponentName(QStringLiteral("android_defaults"));
#endif
        notification->setTitle(title);
        notification->setText(text);

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
