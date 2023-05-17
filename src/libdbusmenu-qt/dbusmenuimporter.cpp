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
#include "dbusmenuimporter.h"

// Qt
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusVariant>
#include <QFont>
#include <QMenu>
#include <QPointer>
#include <QSignalMapper>
#include <QTime>
#include <QTimer>
#include <QToolButton>
#include <QWidgetAction>

// Local
#include "dbusmenushortcut_p.h"
#include "dbusmenutypes_p.h"
#include "debug_p.h"
#include "utils_p.h"

// #define BENCHMARK
#ifdef BENCHMARK
#include <QTime>
static QTime sChrono;
#endif

static const char *DBUSMENU_INTERFACE = "com.canonical.dbusmenu";

static const int ABOUT_TO_SHOW_TIMEOUT = 3000;
static const int REFRESH_TIMEOUT = 4000;

static const char *DBUSMENU_PROPERTY_ID = "_dbusmenu_id";
static const char *DBUSMENU_PROPERTY_ICON_NAME = "_dbusmenu_icon_name";
static const char *DBUSMENU_PROPERTY_ICON_DATA_HASH = "_dbusmenu_icon_data_hash";

static QAction *createKdeTitle(QAction *action, QWidget *parent)
{
    QToolButton *titleWidget = new QToolButton(0);
    QFont font = titleWidget->font();
    font.setBold(true);
    titleWidget->setFont(font);
    titleWidget->setIcon(action->icon());
    titleWidget->setText(action->text());
    titleWidget->setDown(true);
    titleWidget->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QWidgetAction *titleAction = new QWidgetAction(parent);
    titleAction->setDefaultWidget(titleWidget);
    return titleAction;
}

class DBusMenuImporterPrivate
{
public:
    DBusMenuImporter *q;

    QDBusAbstractInterface *m_interface;
    QMenu *m_menu;
    typedef QMap<int, QPointer<QAction>> ActionForId;
    ActionForId m_actionForId;
    QSignalMapper m_mapper;
    QTimer *m_pendingLayoutUpdateTimer;

    QSet<int> m_idsRefreshedByAboutToShow;
    QSet<int> m_pendingLayoutUpdates;

    bool m_mustEmitMenuUpdated;

    DBusMenuImporterType m_type;

    QDBusPendingCallWatcher *refresh(int id)
    {
#ifdef BENCHMARK
        DMDEBUG << "Starting refresh chrono for id" << id;
        sChrono.start();
#endif
        QDBusPendingCall call = m_interface->asyncCall("GetLayout", id, 1, QStringList());
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, q);
        watcher->setProperty(DBUSMENU_PROPERTY_ID, id);
        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)), q, SLOT(slotGetLayoutFinished(QDBusPendingCallWatcher *)));

        return watcher;
    }

    QMenu *createMenu(QWidget *parent)
    {
        QMenu *menu = q->createMenu(parent);
        QObject::connect(menu, SIGNAL(aboutToShow()), q, SLOT(slotMenuAboutToShow()));
        QObject::connect(menu, SIGNAL(aboutToHide()), q, SLOT(slotMenuAboutToHide()));
        return menu;
    }

    /**
     * Init all the immutable action properties here
     * TODO: Document immutable properties?
     *
     * Note: we remove properties we handle from the map (using QMap::take()
     * instead of QMap::value()) to avoid warnings about these properties in
     * updateAction()
     */
    QAction *createAction(int id, const QVariantMap &_map, QWidget *parent)
    {
        QVariantMap map = _map;
        QAction *action = new QAction(parent);
        action->setProperty(DBUSMENU_PROPERTY_ID, id);

        QString type = map.take("type").toString();
        if (type == "separator") {
            action->setSeparator(true);
        }

        if (map.take("children-display").toString() == "submenu") {
            QMenu *menu = createMenu(parent);
            action->setMenu(menu);
        }

        QString toggleType = map.take("toggle-type").toString();
        if (!toggleType.isEmpty()) {
            action->setCheckable(true);
            if (toggleType == "radio") {
                QActionGroup *group = new QActionGroup(action);
                group->addAction(action);
            }
        }

        bool isKdeTitle = map.take("x-kde-title").toBool();
        updateAction(action, map, map.keys());

        if (isKdeTitle) {
            action = createKdeTitle(action, parent);
        }

        return action;
    }

    /**
     * Update mutable properties of an action. A property may be listed in
     * requestedProperties but not in map, this means we should use the default value
     * for this property.
     *
     * @param action the action to update
     * @param map holds the property values
     * @param requestedProperties which properties has been requested
     */
    void updateAction(QAction *action, const QVariantMap &map, const QStringList &requestedProperties)
    {
        Q_FOREACH (const QString &key, requestedProperties) {
            updateActionProperty(action, key, map.value(key));
        }
    }

    void updateActionProperty(QAction *action, const QString &key, const QVariant &value)
    {
        if (key == "label") {
            updateActionLabel(action, value);
        } else if (key == "enabled") {
            updateActionEnabled(action, value);
        } else if (key == "toggle-state") {
            updateActionChecked(action, value);
        } else if (key == "icon-name") {
            updateActionIconByName(action, value);
        } else if (key == "icon-data") {
            updateActionIconByData(action, value);
        } else if (key == "visible") {
            updateActionVisible(action, value);
        } else if (key == "shortcut") {
            updateActionShortcut(action, value);
        } else if (key == "children-display") {
        } else {
            DMWARNING << "Unhandled property update" << key;
        }
    }

    void updateActionLabel(QAction *action, const QVariant &value)
    {
        QString text = swapMnemonicChar(value.toString(), '_', '&');
        action->setText(text);
    }

    void updateActionEnabled(QAction *action, const QVariant &value)
    {
        action->setEnabled(value.isValid() ? value.toBool() : true);
    }

    void updateActionChecked(QAction *action, const QVariant &value)
    {
        if (action->isCheckable() && value.isValid()) {
            action->setChecked(value.toInt() == 1);
        }
    }

    void updateActionIconByName(QAction *action, const QVariant &value)
    {
        QString iconName = value.toString();
        QString previous = action->property(DBUSMENU_PROPERTY_ICON_NAME).toString();
        if (previous == iconName) {
            return;
        }
        action->setProperty(DBUSMENU_PROPERTY_ICON_NAME, iconName);
        if (iconName.isEmpty()) {
            action->setIcon(QIcon());
            return;
        }
        action->setIcon(q->iconForName(iconName));
    }

    void updateActionIconByData(QAction *action, const QVariant &value)
    {
        QByteArray data = value.toByteArray();
        uint dataHash = qHash(data);
        uint previousDataHash = action->property(DBUSMENU_PROPERTY_ICON_DATA_HASH).toUInt();
        if (previousDataHash == dataHash) {
            return;
        }
        action->setProperty(DBUSMENU_PROPERTY_ICON_DATA_HASH, dataHash);
        QPixmap pix;
        if (!pix.loadFromData(data)) {
            DMWARNING << "Failed to decode icon-data property for action" << action->text();
            action->setIcon(QIcon());
            return;
        }
        action->setIcon(QIcon(pix));
    }

    void updateActionVisible(QAction *action, const QVariant &value)
    {
        action->setVisible(value.isValid() ? value.toBool() : true);
    }

    void updateActionShortcut(QAction *action, const QVariant &value)
    {
        QDBusArgument arg = value.value<QDBusArgument>();
        DBusMenuShortcut dmShortcut;
        arg >> dmShortcut;
        QKeySequence keySequence = dmShortcut.toKeySequence();
        action->setShortcut(keySequence);
    }

    QMenu *menuForId(int id) const
    {
        if (id == 0) {
            return q->menu();
        }
        QAction *action = m_actionForId.value(id);
        if (!action) {
            return 0;
        }
        return action->menu();
    }

    void slotItemsPropertiesUpdated(const DBusMenuItemList &updatedList, const DBusMenuItemKeysList &removedList);

    void sendEvent(int id, const QString &eventId)
    {
        QVariant empty = QVariant::fromValue(QDBusVariant(QString()));
        m_interface->asyncCall("Event", id, eventId, empty, 0u);
    }

    bool waitForWatcher(QDBusPendingCallWatcher *_watcher, int maxWait)
    {
        QPointer<QDBusPendingCallWatcher> watcher(_watcher);

        if (m_type == ASYNCHRONOUS) {
            QTimer timer;
            timer.setSingleShot(true);
            QEventLoop loop;
            loop.connect(&timer, SIGNAL(timeout()), SLOT(quit()));
            loop.connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)), SLOT(quit()));
            timer.start(maxWait);
            loop.exec();
            timer.stop();

            if (!watcher) {
                // Watcher died. This can happen if importer got deleted while we were
                // waiting. See:
                // https://bugs.kde.org/show_bug.cgi?id=237156
                return false;
            }

            if (!watcher->isFinished()) {
                // Timed out
                return false;
            }
        } else {
            watcher->waitForFinished();
        }

        if (watcher->isError()) {
            DMWARNING << watcher->error().message();
            return false;
        }

        return true;
    }
};

DBusMenuImporter::DBusMenuImporter(const QString &service, const QString &path, QObject *parent)
    : DBusMenuImporter(service, path, ASYNCHRONOUS, parent)
{
}

DBusMenuImporter::DBusMenuImporter(const QString &service, const QString &path, DBusMenuImporterType type, QObject *parent)
    : QObject(parent)
    , d(new DBusMenuImporterPrivate)
{
    DBusMenuTypes_register();

    d->q = this;
    d->m_interface = new QDBusInterface(service, path, DBUSMENU_INTERFACE, QDBusConnection::sessionBus(), this);
    d->m_menu = 0;
    d->m_mustEmitMenuUpdated = false;

    d->m_type = type;

    connect(&d->m_mapper, SIGNAL(mapped(int)), SLOT(sendClickedEvent(int)));

    d->m_pendingLayoutUpdateTimer = new QTimer(this);
    d->m_pendingLayoutUpdateTimer->setSingleShot(true);
    connect(d->m_pendingLayoutUpdateTimer, SIGNAL(timeout()), SLOT(processPendingLayoutUpdates()));

    // For some reason, using QObject::connect() does not work but
    // QDBusConnect::connect() does
    QDBusConnection::sessionBus().connect(service, path, DBUSMENU_INTERFACE, "LayoutUpdated", "ui", this, SLOT(slotLayoutUpdated(uint, int)));
    QDBusConnection::sessionBus().connect(service,
                                          path,
                                          DBUSMENU_INTERFACE,
                                          "ItemsPropertiesUpdated",
                                          "a(ia{sv})a(ias)",
                                          this,
                                          SLOT(slotItemsPropertiesUpdated(DBusMenuItemList, DBusMenuItemKeysList)));
    QDBusConnection::sessionBus()
        .connect(service, path, DBUSMENU_INTERFACE, "ItemActivationRequested", "iu", this, SLOT(slotItemActivationRequested(int, uint)));

    d->refresh(0);
}

DBusMenuImporter::~DBusMenuImporter()
{
    // Do not use "delete d->m_menu": even if we are being deleted we should
    // leave enough time for the menu to finish what it was doing, for example
    // if it was being displayed.
    d->m_menu->deleteLater();
    delete d;
}

void DBusMenuImporter::slotLayoutUpdated(uint revision, int parentId)
{
    if (d->m_idsRefreshedByAboutToShow.remove(parentId)) {
        return;
    }
    d->m_pendingLayoutUpdates << parentId;
    if (!d->m_pendingLayoutUpdateTimer->isActive()) {
        d->m_pendingLayoutUpdateTimer->start();
    }
}

void DBusMenuImporter::processPendingLayoutUpdates()
{
    QSet<int> ids = d->m_pendingLayoutUpdates;
    d->m_pendingLayoutUpdates.clear();
    Q_FOREACH (int id, ids) {
        d->refresh(id);
    }
}

QMenu *DBusMenuImporter::menu() const
{
    if (!d->m_menu) {
        d->m_menu = d->createMenu(0);
    }
    return d->m_menu;
}

void DBusMenuImporterPrivate::slotItemsPropertiesUpdated(const DBusMenuItemList &updatedList, const DBusMenuItemKeysList &removedList)
{
    Q_FOREACH (const DBusMenuItem &item, updatedList) {
        QAction *action = m_actionForId.value(item.id);
        if (!action) {
            // We don't know this action. It probably is in a menu we haven't fetched yet.
            continue;
        }

        QVariantMap::ConstIterator it = item.properties.constBegin(), end = item.properties.constEnd();
        for (; it != end; ++it) {
            updateActionProperty(action, it.key(), it.value());
        }
    }

    Q_FOREACH (const DBusMenuItemKeys &item, removedList) {
        QAction *action = m_actionForId.value(item.id);
        if (!action) {
            // We don't know this action. It probably is in a menu we haven't fetched yet.
            continue;
        }

        Q_FOREACH (const QString &key, item.properties) {
            updateActionProperty(action, key, QVariant());
        }
    }
}

void DBusMenuImporter::slotItemActivationRequested(int id, uint /*timestamp*/)
{
    QAction *action = d->m_actionForId.value(id);
    DMRETURN_IF_FAIL(action);
    actionActivationRequested(action);
}

void DBusMenuImporter::slotGetLayoutFinished(QDBusPendingCallWatcher *watcher)
{
    int parentId = watcher->property(DBUSMENU_PROPERTY_ID).toInt();
    watcher->deleteLater();

    QDBusPendingReply<uint, DBusMenuLayoutItem> reply = *watcher;
    if (!reply.isValid()) {
        DMWARNING << reply.error().message();
        return;
    }

#ifdef BENCHMARK
    DMDEBUG << "- items received:" << sChrono.elapsed() << "ms";
#endif
    DBusMenuLayoutItem rootItem = reply.argumentAt<1>();

    QMenu *menu = d->menuForId(parentId);
    if (!menu) {
        DMWARNING << "No menu for id" << parentId;
        return;
    }

    menu->clear();

    Q_FOREACH (const DBusMenuLayoutItem &dbusMenuItem, rootItem.children) {
        QAction *action = d->createAction(dbusMenuItem.id, dbusMenuItem.properties, menu);
        DBusMenuImporterPrivate::ActionForId::Iterator it = d->m_actionForId.find(dbusMenuItem.id);
        if (it == d->m_actionForId.end()) {
            d->m_actionForId.insert(dbusMenuItem.id, action);
        } else {
            delete *it;
            *it = action;
        }
        menu->addAction(action);

        connect(action, SIGNAL(triggered()), &d->m_mapper, SLOT(map()));
        d->m_mapper.setMapping(action, dbusMenuItem.id);

        if (action->menu()) {
            d->refresh(dbusMenuItem.id)->waitForFinished();
        }
    }
#ifdef BENCHMARK
    DMDEBUG << "- Menu filled:" << sChrono.elapsed() << "ms";
#endif
}

void DBusMenuImporter::sendClickedEvent(int id)
{
    d->sendEvent(id, QString("clicked"));
}

void DBusMenuImporter::updateMenu()
{
    d->m_mustEmitMenuUpdated = true;
    QMetaObject::invokeMethod(menu(), "aboutToShow");
}

void DBusMenuImporter::slotMenuAboutToShow()
{
    QMenu *menu = qobject_cast<QMenu *>(sender());
    Q_ASSERT(menu);

    QAction *action = menu->menuAction();
    Q_ASSERT(action);

    int id = action->property(DBUSMENU_PROPERTY_ID).toInt();

#ifdef BENCHMARK
    QTime time;
    time.start();
#endif

    QDBusPendingCall call = d->m_interface->asyncCall("AboutToShow", id);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    watcher->setProperty(DBUSMENU_PROPERTY_ID, id);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)), SLOT(slotAboutToShowDBusCallFinished(QDBusPendingCallWatcher *)));

    QPointer<QObject> guard(this);

    if (!d->waitForWatcher(watcher, ABOUT_TO_SHOW_TIMEOUT)) {
        DMWARNING << "Application did not answer to AboutToShow() before timeout";
    }

#ifdef BENCHMARK
    DMVAR(time.elapsed());
#endif
    // "this" got deleted during the call to waitForWatcher(), get out
    if (!guard) {
        return;
    }

    if (menu == d->m_menu && d->m_mustEmitMenuUpdated) {
        d->m_mustEmitMenuUpdated = false;
        menuUpdated();
    }
    if (menu == d->m_menu) {
        menuReadyToBeShown();
    }

    d->sendEvent(id, QString("opened"));
}

void DBusMenuImporter::slotAboutToShowDBusCallFinished(QDBusPendingCallWatcher *watcher)
{
    int id = watcher->property(DBUSMENU_PROPERTY_ID).toInt();
    watcher->deleteLater();

    QDBusPendingReply<bool> reply = *watcher;
    if (reply.isError()) {
        DMWARNING << "Call to AboutToShow() failed:" << reply.error().message();
        return;
    }
    bool needRefresh = reply.argumentAt<0>();

    QMenu *menu = d->menuForId(id);
    DMRETURN_IF_FAIL(menu);

    if (needRefresh || menu->actions().isEmpty()) {
        d->m_idsRefreshedByAboutToShow << id;
        QDBusPendingCallWatcher *watcher2 = d->refresh(id);
        if (!d->waitForWatcher(watcher2, REFRESH_TIMEOUT)) {
            DMWARNING << "Application did not refresh before timeout";
        }
    }
}

void DBusMenuImporter::slotMenuAboutToHide()
{
    QMenu *menu = qobject_cast<QMenu *>(sender());
    Q_ASSERT(menu);

    QAction *action = menu->menuAction();
    Q_ASSERT(action);

    int id = action->property(DBUSMENU_PROPERTY_ID).toInt();
    d->sendEvent(id, QString("closed"));
}

QMenu *DBusMenuImporter::createMenu(QWidget *parent)
{
    return new QMenu(parent);
}

QIcon DBusMenuImporter::iconForName(const QString & /*name*/)
{
    return QIcon();
}

#include "dbusmenuimporter.moc"
