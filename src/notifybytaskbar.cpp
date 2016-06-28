/*
   Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.

 */


#include "notifybytaskbar.h"
#include "knotifyconfig.h"
#include "knotification.h"
#include "debug_p.h"

#include <QApplication>
#include <QWidget>

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

