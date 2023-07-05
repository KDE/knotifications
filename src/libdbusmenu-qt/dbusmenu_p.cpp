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
#include "dbusmenu_p.h"

// Qt
#include <QAction>
#include <QActionEvent>
#include <QMenu>

// Local
#include "dbusmenuexporter.h"
#include "dbusmenuexporterprivate_p.h"
#include "debug_p.h"

DBusMenu::DBusMenu(QMenu *menu, DBusMenuExporter *exporter, int parentId)
    : QObject(menu)
    , m_exporter(exporter)
    , m_parentId(parentId)
{
    menu->installEventFilter(this);
    connect(m_exporter, SIGNAL(destroyed(QObject *)), SLOT(deleteMe()));
}

DBusMenu::~DBusMenu()
{
}

bool DBusMenu::eventFilter(QObject *, QEvent *event)
{
    QActionEvent *actionEvent = nullptr;
    switch (event->type()) {
    case QEvent::ActionAdded:
    case QEvent::ActionChanged:
    case QEvent::ActionRemoved:
        actionEvent = static_cast<QActionEvent *>(event);
        break;
    default:
        return false;
    }
    switch (event->type()) {
    case QEvent::ActionAdded:
        addAction(actionEvent->action());
        break;
    case QEvent::ActionChanged:
        updateAction(actionEvent->action());
        break;
    case QEvent::ActionRemoved:
        removeAction(actionEvent->action());
        break;
    default:
        break;
    }
    return false;
}

void DBusMenu::addAction(QAction *action)
{
    m_exporter->d->addAction(action, m_parentId);
}

void DBusMenu::updateAction(QAction *action)
{
    m_exporter->d->updateAction(action);
}

void DBusMenu::removeAction(QAction *action)
{
    m_exporter->d->removeAction(action, m_parentId);
}

void DBusMenu::deleteMe()
{
    delete this;
}

#include "moc_dbusmenu_p.cpp"
