/*
    Copyright (C) 2019 Piyush Aggarwal <piyushaggarwal002@gmail.com>

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
    void close(KNotification * notification) override;
    void update(KNotification *notification, KNotifyConfig *config) override;
private:
    QHash<int, QPointer<KNotification>> m_notifications;
    QString m_program = QStringLiteral("SnoreToast.exe");
    QLocalServer m_server;
    QTemporaryDir m_iconDir;
};

#endif // NOTIFYBYSNORE_H
