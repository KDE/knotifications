/* This file is part of the dbusmenu-qt library
   Copyright 2010 Canonical
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
#ifndef DBUSMENUEXPORTERDBUS_P_H
#define DBUSMENUEXPORTERDBUS_P_H

// Local
#include <dbusmenutypes_p.h>

// Qt
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusVariant>

class DBusMenuExporter;

/**
 * Internal class implementing the DBus side of DBusMenuExporter
 * This avoid exposing the implementation of the DBusMenu spec to the outside
 * world.
 */
class DBusMenuExporterDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.canonical.dbusmenu")
    Q_PROPERTY(uint Version READ Version)
    Q_PROPERTY(QString Status READ status)
public:
    DBusMenuExporterDBus(DBusMenuExporter *m_exporter);

    uint Version() const
    {
        return 2;
    }

    QString status() const;
    void setStatus(const QString &status);

public Q_SLOTS:
    Q_NOREPLY void Event(int id, const QString &eventId, const QDBusVariant &data, uint timestamp);
    QDBusVariant GetProperty(int id, const QString &property);
    uint GetLayout(int parentId, int recursionDepth, const QStringList &propertyNames, DBusMenuLayoutItem &item);
    DBusMenuItemList GetGroupProperties(const QList<int> &ids, const QStringList &propertyNames);
    bool AboutToShow(int id);

Q_SIGNALS:
    void ItemsPropertiesUpdated(DBusMenuItemList, DBusMenuItemKeysList);
    void LayoutUpdated(uint revision, int parentId);
    void ItemActivationRequested(int id, uint timeStamp);

private:
    DBusMenuExporter *m_exporter;
    QString m_status;

    friend class DBusMenuExporter;
    friend class DBusMenuExporterPrivate;

    QVariantMap getProperties(int id, const QStringList &names) const;
};

#endif /* DBUSMENUEXPORTERDBUS_P_H */
