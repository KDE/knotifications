/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NOTIFYBYLOGFILE_H
#define NOTIFYBYLOGFILE_H

#include "knotificationplugin.h"

class KNotification;

class NotifyByLogfile : public KNotificationPlugin
{
    Q_OBJECT

public:
    explicit NotifyByLogfile(QObject *parent = nullptr);
    ~NotifyByLogfile() override;

    QString optionName() override
    {
        return QStringLiteral("Logfile");
    }
    void notify(KNotification *notification, KNotifyConfig *config) override;
};

#endif
