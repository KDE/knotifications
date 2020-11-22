/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "notifybytaskbar.h"
#include "knotifyconfig.h"
#include "knotification.h"
#include "debug_p.h"

#include <QApplication>
#include <QTimer>

NotifyByTaskbar::NotifyByTaskbar(QObject *parent)
    : KNotificationPlugin(parent)
{
}

NotifyByTaskbar::~NotifyByTaskbar()
{
}

void NotifyByTaskbar::notify(KNotification *notification, KNotifyConfig *config)
{
    // HACK We cannot immediately close the notification from notify()
    // if we do it then it won't have a proper id yet and it can't be cleaned up correctly.
    // The id is only assigned after notify has finished
    QTimer::singleShot(0, this, [this, notification, config]{
        Q_UNUSED(config);
        if (!notification->widget()) {
            qCWarning(LOG_KNOTIFICATIONS) << "Could not notify " << notification->eventId() << "by taskbar, notification has no associated widget";

            finish(notification);
            return;
        }

        QApplication::alert(notification->widget());

        finish(notification);
    });
}

