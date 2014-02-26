/*
   Copyright (C) 2010 by Sjors Gielen <dazjorz@dazjorz.com>

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

#include "notifybypopupgrowl.h"
#include <QFile>
#include <QSystemTrayIcon>

#define GROWL_LOCATION_MACOSX "/Library/PreferencePanes/Growl.prefPane/" \
                              "Contents/MacOS/Growl"
#define GROWL_LOCATION_WIN32 "C:/Program Files/Growl for Windows/Growl.exe"

/**
 * @brief Check if Growl can display plugins.
 * Currently, this checks only if Growl is installed, not if it's running.
 * As soon as the Growl Notification Protocol is finished, it will be
 * implemented and used for this check.
 */
bool NotifyByPopupGrowl::canPopup()
{
  return QFile::exists( GROWL_LOCATION_MACOSX )
      || QFile::exists( GROWL_LOCATION_WIN32  );
}

/**
 * @brief Get the capabilities supported by Growl.
 */
QStringList NotifyByPopupGrowl::capabilities()
{
  return QStringList();
}

/**
 * @brief Send a popup through Growl.
 * @param icon The icon inside the notification. Currently ignored.
 * @param timeout The time in ms to show the notification.
 * @param title The title displayed inside the notification.
 * @param message The message displayed inside the notification.
 */
void NotifyByPopupGrowl::popup( const QPixmap *icon, int timeout,
                                const QString &title, const QString &message )
{
  Q_UNUSED( icon );

  QSystemTrayIcon i;
  i.show();
  i.showMessage( title, message,
                 QSystemTrayIcon::Information, timeout );
  i.hide();
}
