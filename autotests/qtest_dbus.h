/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KNOTIFICATIONS_QTEST_DBUS_H
#define KNOTIFICATIONS_QTEST_DBUS_H

#include <QProcess>
#include <QTest>

#include <stdlib.h>

/* clang-format off */
#define QTEST_GUILESS_MAIN_SYSTEM_DBUS(TestObject) \
    int main(int argc, char *argv[]) \
    { \
        QProcess dbus; \
        dbus.start(QStringLiteral("dbus-launch")); \
        dbus.waitForFinished(10000); \
        QByteArray session = dbus.readLine(); \
        if (session.isEmpty()) { \
            qFatal("Couldn't execute new dbus session"); \
        } \
        int pos = session.indexOf('='); \
        qputenv("DBUS_SESSION_BUS_ADDRESS", session.right(session.count() - pos - 1).trimmed().constData()); \
        session = dbus.readLine(); \
        pos = session.indexOf('='); \
        QByteArray pid = session.right(session.count() - pos - 1).trimmed(); \
        QCoreApplication app(argc, argv); \
        app.setApplicationName(QLatin1String("qttest")); \
        TestObject tc; \
        int result = QTest::qExec(&tc, argc, argv); \
        dbus.start(QStringLiteral("kill"), QStringList() << QStringLiteral("-9") << QString::fromLatin1(pid)); \
        dbus.waitForFinished(); \
        return result; \
    }
/* clang-format on */
#endif // KNOTIFICATIONS_QTEST_DBUS_H
