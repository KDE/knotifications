/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "notifybytaskbar.h"
#include "debug_p.h"
#include "knotification.h"
#include "knotifyconfig.h"

#include <QApplication>

NotifyByTaskbar::NotifyByTaskbar(QObject *parent)
    : KNotificationPlugin(parent)
{
}

NotifyByTaskbar::~NotifyByTaskbar()
{
}

void NotifyByTaskbar::notify(KNotification *notification, KNotifyConfig *config)
{
    Q_UNUSED(config);
    if (!notification->widget()) {
        qCWarning(LOG_KNOTIFICATIONS) << "Could not notify " << notification->eventId() << "by taskbar, notification has no associated widget";
        finish(notification);
        return;
    }

    QApplication::alert(notification->widget());

    finish(notification);
}
