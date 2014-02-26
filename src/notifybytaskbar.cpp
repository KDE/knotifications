/*
   Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>


   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

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
