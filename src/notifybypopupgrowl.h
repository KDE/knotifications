/*
   Copyright (C) 2010 by Sjors Gielen <dazjorz@dazjorz.com>

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

#ifndef NOTIFYBYPOPUPGROWL_H
#define NOTIFYBYPOPUPGROWL_H

#include <QPixmap>
#include <QString>

/**
 * @brief Display a notification using Growl.
 *
 * Currently, this class uses QSystemTrayIcon to actually display the
 * notification. Inside the Growl project, a protocol is being developed
 * to display notifications; this protocol is currently only implemented
 * (partly) in the Windows version of Growl. Once it is finished, it will be
 * implemented in KNotify and used instead.
 * (The normal Growl API is written in Objective C. It's possible to use it,
 * but it's a lot harder than just waiting for GNTP to stabilize.)
 */
class NotifyByPopupGrowl
{
public:
    static bool canPopup();
    static QStringList capabilities();
    static void popup(const QPixmap *icon, int timeout,
                      const QString &title, const QString &message);
};

#endif // NOTIFYBYPOPUPGROWL_H
