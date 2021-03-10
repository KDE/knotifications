/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NOTIFYBYTASKBAR_H
#define NOTIFYBYTASKBAR_H

#include "knotificationplugin.h"

class NotifyByTaskbar : public KNotificationPlugin
{
    Q_OBJECT
public:
    explicit NotifyByTaskbar(QObject *parent = nullptr);
    ~NotifyByTaskbar() override;

    QString optionName() override
    {
        return QStringLiteral("Taskbar");
    }
    void notify(KNotification *notification, KNotifyConfig *config) override;
};

#endif
