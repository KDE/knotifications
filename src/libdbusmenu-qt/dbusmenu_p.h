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
#ifndef DBUSMENU_H
#define DBUSMENU_H

#include <QEvent>
#include <QObject>

class QAction;
class QMenu;

class DBusMenuExporter;

/**
 * Internal class responsible for tracking changes in a menu and reporting them
 * through DBusMenuExporter
 * @internal
 */
class DBusMenu : public QObject
{
    Q_OBJECT
public:
    DBusMenu(QMenu *menu, DBusMenuExporter *exporter, int parentId);
    ~DBusMenu() override;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private Q_SLOTS:
    void deleteMe();

private:
    void addAction(QAction *action);
    void updateAction(QAction *action);
    void removeAction(QAction *action);

    DBusMenuExporter *const m_exporter;
    const int m_parentId;
};

#endif /* DBUSMENU_H */
