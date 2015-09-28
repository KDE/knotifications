/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart at kde.org>
   Copyright (C) 2006 Thiago Macieira <thiago@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KNOTIFICATIONMANAGER_H
#define KNOTIFICATIONMANAGER_H

#include <knotification.h>

class KNotification;
class QPixmap;
class QStringList;
class KNotificationPlugin;

/**
 * @internal
 * @author Olivier Goffart
 */
class KNotificationManager : public QObject
{
    Q_OBJECT
public:
    static KNotificationManager *self();
    ~KNotificationManager();

    void addPlugin(KNotificationPlugin *notifyPlugin);

    /**
     * send the dbus call to the knotify server
     */
    int notify(KNotification *n);

    /**
     * send the close dcop call to the knotify server for the notification with the identifier @p id .
     * And remove the notification from the internal map
     * @param id the id of the notification
     * @param force if false, only close registered notification
     */
    void close(int id, bool force = false);

    /**
     * update one notification text and pixmap and actions
     */
    void update(KNotification *n);

    /**
     * re-emit the notification, eventually with new contexts
     */
    void reemit(KNotification *n);

private Q_SLOTS:
    void notificationClosed();
    void notificationActivated(int id,  int action);
    void notifyPluginFinished(KNotification *notification);
    void reparseConfiguration(const QString &app);

private:
    struct Private;
    Private *const d;
    KNotificationManager();

    friend class KNotificationManagerSingleton;
};

#endif
