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

#include <QDebug>
#include <QWidget>

#include <KWindowSystem>

NotifyByTaskbar::NotifyByTaskbar(QObject *parent)
    : KNotifyPlugin(parent)
{
}

NotifyByTaskbar::~NotifyByTaskbar()
{
}

void NotifyByTaskbar::notify(KNotification *notification, KNotifyConfig *config)
{
    qDebug() << notification->id() << notification->widget()->topLevelWidget()->winId();

    WId win = notification->widget()->topLevelWidget()->winId();
    if (win != 0) {
        KWindowSystem::demandAttention(win);
    }

    finish(notification);
}

#include "notifybytaskbar.moc"
