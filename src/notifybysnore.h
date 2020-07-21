/*
    SPDX-FileCopyrightText: 2019 Piyush Aggarwal <piyushaggarwal002@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef NOTIFYBYSNORE_H
#define NOTIFYBYSNORE_H

#include "knotificationplugin.h"

#include <QPointer>
#include <QLocalServer>
#include <QProcess>
#include <QString>
#include <QTemporaryDir>

/** Windows notification backend - inspired by Android notification backend. */
class NotifyBySnore : public KNotificationPlugin
{
    Q_OBJECT

public:
    explicit NotifyBySnore(QObject *parent = nullptr);
    ~NotifyBySnore() override;

    QString optionName() override { return QStringLiteral("Popup"); }
    void notify(KNotification *notification, KNotifyConfig *config) override;
    void notifyDeferred(KNotification* notification);
    void close(KNotification * notification) override;
    void update(KNotification *notification, KNotifyConfig *config) override;
private:
    QHash<int, QPointer<KNotification>> m_notifications;
    QString m_program = QStringLiteral("SnoreToast.exe");
    QLocalServer m_server;
    QTemporaryDir m_iconDir;
};

#endif // NOTIFYBYSNORE_H
