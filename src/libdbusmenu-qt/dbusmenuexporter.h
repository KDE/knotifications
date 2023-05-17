/* This file is part of the dbusmenu-qt library
   Copyright 2009 Canonical
   Author: Aurelien Gateau <aurelien.gateau@canonical.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef DBUSMENUEXPORTER_H
#define DBUSMENUEXPORTER_H

// Qt
#include <QtCore/QObject>
#include <QtDBus/QDBusConnection>

// Local
#include <dbusmenu_export.h>

class QAction;
class QMenu;

class DBusMenuExporterPrivate;

/**
 * A DBusMenuExporter instance can serialize a menu over DBus
 */
class DBUSMENU_EXPORT DBusMenuExporter : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a DBusMenuExporter exporting menu at the dbus object path
     * dbusObjectPath, using the given dbusConnection.
     * The instance adds itself to the menu children.
     */
    DBusMenuExporter(const QString &dbusObjectPath, QMenu *menu, const QDBusConnection &dbusConnection = QDBusConnection::sessionBus());

    virtual ~DBusMenuExporter();

    /**
     * Asks the matching DBusMenuImporter to activate @p action. For menus it
     * means popup them, for items it means triggering the associated action.
     */
    void activateAction(QAction *action);

    /**
     * The status of the menu. Can be one of "normal" or "notice". This can be
     * used to notify the other side the menu should be made more visible.
     * For example, appmenu uses it to tell Unity panel to show/hide the menubar
     * when the Alt modifier is pressed/released.
     */
    void setStatus(const QString &status);

    /**
     * Returns the status of the menu.
     * @ref setStatus
     */
    QString status() const;

protected:
    /**
     * Must extract the icon name for action. This is the name which will
     * be used to present the icon over DBus.
     * Default implementation returns action->icon().name() when built on Qt
     * >= 4.7 and a null string otherwise.
     */
    virtual QString iconNameForAction(QAction *action);

private Q_SLOTS:
    void doUpdateActions();
    void doEmitLayoutUpdated();
    void slotActionDestroyed(QObject *);

private:
    Q_DISABLE_COPY(DBusMenuExporter)
    DBusMenuExporterPrivate *const d;

    friend class DBusMenuExporterPrivate;
    friend class DBusMenuExporterDBus;
    friend class DBusMenu;
};

#endif /* DBUSMENUEXPORTER_H */
