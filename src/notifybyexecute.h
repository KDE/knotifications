/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NOTIFYBYEXECUTE_H
#define NOTIFYBYEXECUTE_H

#include "knotificationplugin.h"

class NotifyByExecute : public KNotificationPlugin
{
    Q_OBJECT
public:
    explicit NotifyByExecute(QObject *parent = nullptr);
    ~NotifyByExecute() override;

    QString optionName() override
    {
        return QStringLiteral("Execute");
    }
    void notify(KNotification *notification, KNotifyConfig *config) override;
};

#endif
