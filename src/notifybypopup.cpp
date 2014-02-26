/*
   Copyright (C) 2005-2009 by Olivier Goffart <ogoffart at kde.org>
   Copyright (C) 2008 by Dmitry Suzdalev <dimsuz@gmail.com>

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

#include "notifybypopup.h"
#include "imageconverter.h"
#include "notifybypopupgrowl.h"

#include <kdebug.h>
#include <kpassivepopup.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <khbox.h>
#include <kvbox.h>
#include <kcharsets.h>
#include <knotifyconfig.h>

#include <QBuffer>
#include <QImage>
#include <QLabel>
#include <QTextDocument>
#include <QApplication>
#include <QDesktopWidget>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QXmlStreamReader>
#include <kconfiggroup.h>

static const char dbusServiceName[] = "org.freedesktop.Notifications";
static const char dbusInterfaceName[] = "org.freedesktop.Notifications";
static const char dbusPath[] = "/org/freedesktop/Notifications";

NotifyByPopup::NotifyByPopup(QObject *parent) 
  : KNotifyPlugin(parent),
    m_animationTimer(0),
    m_dbusServiceExists(false),
    m_dbusServiceCapCacheDirty(true)
{
    QRect screen = QApplication::desktop()->availableGeometry();
    m_nextPosition = screen.top();

    // check if service already exists on plugin instantiation
    QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
    m_dbusServiceExists = interface && interface->isServiceRegistered(dbusServiceName);

    if (m_dbusServiceExists) {
        onServiceOwnerChanged(dbusServiceName, QString(), "_"); //connect signals
    }

    // to catch register/unregister events from service in runtime
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(this);
    watcher->setConnection(QDBusConnection::sessionBus());
    watcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    watcher->addWatchedService(dbusServiceName);
    connect(watcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            SLOT(onServiceOwnerChanged(QString,QString,QString)));

    if (!m_dbusServiceExists) {
        bool startfdo = false;
#ifdef Q_WS_WIN
        startfdo = true;
#else
        if (qgetenv("KDE_FULL_SESSION").isEmpty()) {
            QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.DBus",
                            "/org/freedesktop/DBus",
                            "org.freedesktop.DBus",
                            "ListActivatableNames");

            // FIXME - sync DBus call...though it runs in its own process, probably no harm
            QDBusReply<QStringList> reply = QDBusConnection::sessionBus().call(message);
            if (reply.isValid() && reply.value().contains(dbusServiceName)) {
                startfdo = true;
                // We need to set m_dbusServiceExists to true because dbus might be too slow
                // starting the service and the first call to NotifyByPopup::notify
                // might not have had the service up, by setting this to true we
                // guarantee it will still go through dbus and dbus will do the correct
                // thing and wait for the service to go up
                m_dbusServiceExists = true;
            }
        }
#endif
        if (startfdo) {
            QDBusConnection::sessionBus().interface()->startService(dbusServiceName);
        }
    }
}


NotifyByPopup::~NotifyByPopup()
{
    Q_FOREACH (KPassivePopup *p, m_passivePopups) {
        p->deleteLater();
    }
}

void NotifyByPopup::notify(int id, KNotifyConfig *config)
{
    kDebug() << id << "  active notifications:" << m_passivePopups.keys() << m_idMap.keys();

    if (m_passivePopups.contains(id) || m_idMap.contains(id)) {
        kDebug() << "the popup is already shown";
        finish(id);
        return;
    }

    // if Notifications DBus service exists on bus,
    // it'll be used instead
    if (m_dbusServiceExists) {
        if (!sendNotificationDBus(id, 0, config)) {
            finish(id); //an error ocurred.
        }
        return;
    }

    // Default to 6 seconds if no timeout has been defined
    int timeout = config->timeout == -1 ? 6000 : config->timeout;

    // if Growl can display our popups, use that instead
    if (NotifyByPopupGrowl::canPopup()) {
        KNotifyConfig *c = ensurePopupCompatibility(config);

        QString appCaption, iconName;
        getAppCaptionAndIconName(c, &appCaption, &iconName);

        KIconLoader iconLoader(iconName);
        QPixmap appIcon = iconLoader.loadIcon(iconName, KIconLoader::Small);

        NotifyByPopupGrowl::popup(&appIcon, timeout, appCaption, c->text);

        // Finish immediately, because current NotifyByPopupGrowl can't callback
        finish(id);
        delete c;
        return;
    }

    KPassivePopup *pop = new KPassivePopup(config->winId);
    m_passivePopups[id] = pop;
    fillPopup(pop, id, config);

    QRect screen = QApplication::desktop()->availableGeometry();
    pop->setAutoDelete(true);
    connect(pop, SIGNAL(destroyed()), this, SLOT(onPassivePopupDestroyed()));

    pop->setTimeout(timeout);
    pop->adjustSize();
    pop->show(QPoint(screen.left() + screen.width()/2 - pop->width()/2 , m_nextPosition));
    m_nextPosition += pop->height();
}

void NotifyByPopup::slotPopupDestroyed()
{
    const QObject *destroyedPopup = sender();
    if (!destroyedPopup) {
        return;
    }

    QMap<int, KPassivePopup*>::iterator it;
    for (it = m_passivePopups.begin(); it != m_passivePopups.end(); ++it) {
        QObject *popup = it.value();
        if (popup && popup == destroyedPopup) {
            finish(it.key());
            m_passivePopups.remove(it.key());
            break;
        }
    }

    //relocate popup
    if (!m_animationTimer) {
        m_animationTimer = startTimer(10);
    }
}

void NotifyByPopup::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != m_animationTimer) {
        return KNotifyPlugin::timerEvent(event);
    }

    bool cont = false;
    QRect screen = QApplication::desktop()->availableGeometry();
    m_nextPosition = screen.top();

    Q_FOREACH (KPassivePopup *popup, m_passivePopups)
    {
        int y = popup->pos().y();
        if (y > m_nextPosition) {
            y = qMax(y - 5, m_nextPosition);
            m_nextPosition = y + popup->height();
            cont = cont || y != m_nextPosition;
            popup->move(popup->pos().x(), y);
        } else {
            m_nextPosition += popup->height();
        }
    }

    if (!cont) {
        killTimer(m_animationTimer);
        m_animationTimer = 0;
    }
}

void NotifyByPopup::slotLinkClicked(const QString &adr)
{
    unsigned int id = adr.section('/' , 0 , 0).toUInt();
    unsigned int action = adr.section('/' , 1 , 1).toUInt();

//    kDebug() << id << " " << action;

    if (id == 0 || action == 0) {
        return;
    }

    emit actionInvoked(id, action);
}

void NotifyByPopup::close(int id)
{
    delete m_passivePopups.take(id);

    if (m_dbusServiceExists) {
        closeNotificationDBus(id);
    }
}

void NotifyByPopup::update(int id, KNotifyConfig *config)
{
    if (m_passivePopups.contains(id)) {
        KPassivePopup *p = m_passivePopups[id];
        fillPopup(p, id, config);
        return;
    }

    // if Notifications DBus service exists on bus,
    // it'll be used instead
    if (m_dbusServiceExists) {
        sendNotificationDBus(id, id, config);
        return;
    }

    // otherwise, just display a new Growl notification
    if (NotifyByPopupGrowl::canPopup()) {
        notify(id, config);
    }
}

void NotifyByPopup::fillPopup(KPassivePopup *popup, int id, KNotifyConfig *config)
{
    QString appCaption;
    QString iconName;
    getAppCaptionAndIconName(config, &appCaption, &iconName);

    KIconLoader iconLoader(iconName);
    QPixmap appIcon = iconLoader.loadIcon(iconName, KIconLoader::Small);

    KVBox *vb = popup->standardView(config->title.isEmpty() ? appCaption : config->title,
                                    config->image.isNull() ? config->text : QString(),
                                    appIcon);

    if (!config->image.isNull()) {
        QPixmap popupIcon = QPixmap::fromImage(config->image.toImage());
        KHBox *hb = new KHBox(vb);
        hb->setSpacing(KDialog::spacingHint());

        QLabel *pil = new QLabel(hb);
        pil->setPixmap(pix);
        pil->setScaledContents(true);

        if (popupIcon.height() > 80 && popupIcon.height() > popupIcon.width()) {
            pil->setMaximumHeight(80);
            pil->setMaximumWidth(80 * pix.width() / pix.height());
        } else if(popupIcon.width() > 80 && popupIcon.height() <= popupIcon.width()) {
            pil->setMaximumWidth(80);
            pil->setMaximumHeight(80*pix.height()/pix.width());
        }

        KVBox *vb2 = new KVBox(hb);
        QLabel *msg = new QLabel(config->text, vb2);
        msg->setAlignment(Qt::AlignLeft);
    }


    if ( !config->actions.isEmpty() )
    {
        QString linkCode=QString::fromLatin1("<p align=\"right\">");
        int i=0;
        foreach ( const QString & it , config->actions )
        {
            i++;
            linkCode+=QString::fromLatin1("&nbsp;<a href=\"%1/%2\">%3</a> ").arg(id, i, it.toHtmlEscaped());
        }
        linkCode+=QString::fromLatin1("</p>");
        QLabel *link = new QLabel(linkCode , vb );
        link->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        link->setOpenExternalLinks(false);
        //link->setAlignment( AlignRight );
        QObject::connect(link, SIGNAL(linkActivated(const QString &)), this, SLOT(onPassivePopupLinkClicked(const QString& ) ) );
        QObject::connect(link, SIGNAL(linkActivated(const QString &)), popup, SLOT(hide()));
    }

    popup->setView( vb );
}

void NotifyByPopup::onServiceOwnerChanged( const QString & serviceName,
        const QString & oldOwner, const QString & newOwner )
{
    kDebug() << serviceName << oldOwner << newOwner;
    // tell KNotify that all existing notifications which it sent
    // to DBus had been closed
    foreach (int id, m_idMap.keys())
        finished(id);
    m_idMap.clear();

    m_dbusServiceCapCacheDirty = true;
    m_dbusServiceCapabilities.clear();

    if(newOwner.isEmpty())
    {
        m_dbusServiceExists = false;
    }
    else if(oldOwner.isEmpty())
    {
        m_dbusServiceExists = true;

        // connect to action invocation signals
        bool connected = QDBusConnection::sessionBus().connect(QString(), // from any service
                dbusPath,
                dbusInterfaceName,
                "ActionInvoked",
                this,
                SLOT(slotDBusNotificationActionInvoked(uint,const QString&)));
        if (!connected) {
            kWarning() << "warning: failed to connect to ActionInvoked dbus signal";
        }

        connected = QDBusConnection::sessionBus().connect(QString(), // from any service
                dbusPath,
                dbusInterfaceName,
                "NotificationClosed",
                this,
                SLOT(slotDBusNotificationClosed(uint,uint)));
        if (!connected) {
            kWarning() << "warning: failed to connect to NotificationClosed dbus signal";
        }
    }
}


void NotifyByPopup::slotDBusNotificationActionInvoked(uint dbus_id, const QString& actKey)
{
    // find out knotify id
    int id = m_idMap.key(dbus_id, 0);
    if (id == 0) {
        kDebug() << "failed to find knotify id for dbus_id" << dbus_id;
        return;
    }
    kDebug() << "action" << actKey << "invoked for notification " << id;
    // emulate link clicking
    onPassivePopupLinkClicked( QString("%1/%2").arg(id).arg(actKey) );
    // now close notification - similar to popup behaviour
    // (popups are hidden after link activation - see 'connects' of linkActivated signal above)
    closeNotificationDBus(id);
}

void NotifyByPopup::slotDBusNotificationClosed(uint dbus_id, uint reason)
{
    Q_UNUSED(reason)
    // find out knotify id
    int id = m_idMap.key(dbus_id, 0);
    kDebug() << dbus_id << "  -> " << id;
    if (id == 0) {
        kDebug() << "failed to find knotify id for dbus_id" << dbus_id;
        return;
    }
    // tell KNotify that this notification has been closed
    m_idMap.remove(id);
    finished(id);
}

void NotifyByPopup::getAppCaptionAndIconName(KNotifyConfig *config, QString *appCaption, QString *iconName)
{
    KConfigGroup globalgroup(&(*config->eventsfile), "Global");
    *appCaption = globalgroup.readEntry("Name", globalgroup.readEntry("Comment", config->appname));

    KConfigGroup eventGroup(&(*config->eventsfile), QString("Event/%1").arg(config->eventid));
    if (eventGroup.hasKey("IconName")) {
        *iconName = eventGroup.readEntry("IconName", config->appname);
    } else {
        *iconName = globalgroup.readEntry("IconName", config->appname);
    }
}

bool NotifyByPopup::sendNotificationDBus(int id, int replacesId, KNotifyConfig* config_nocheck)
{
    // figure out dbus id to replace if needed
    uint dbus_replaces_id = 0;
    if (replacesId != 0 ) {
        dbus_replaces_id = m_idMap.value(replacesId, 0);
        if (!dbus_replaces_id)
            return false;  //the popup has been closed, there is nothing to replace.
    }

    QDBusMessage m = QDBusMessage::createMethodCall( dbusServiceName, dbusPath, dbusInterfaceName, "Notify" );

    QList<QVariant> args;

    QString appCaption, iconName;
    getAppCaptionAndIconName(config_nocheck, &appCaption, &iconName);

    KNotifyConfig *config = ensurePopupCompatibility( config_nocheck );

    args.append( appCaption ); // app_name
    args.append( dbus_replaces_id ); // replaces_id
    args.append( iconName ); // app_icon
    args.append( config->title.isEmpty() ? appCaption : config->title ); // summary
    args.append( config->text ); // body
    // galago spec defines action list to be list like
    // (act_id1, action1, act_id2, action2, ...)
    //
    // assign id's to actions like it's done in fillPopup() method
    // (i.e. starting from 1)
    QStringList actionList;
    int actId = 0;
    foreach (const QString& actName, config->actions) {
        actId++;
        actionList.append(QString::number(actId));
        actionList.append(actName);
    }

    args.append( actionList ); // actions

    QVariantMap map;
    // Add the application name to the hints.
    // According to fdo spec, the app_name is supposed to be the applicaton's "pretty name"
    // but in some places it's handy to know the application name itself
    if (!config->appname.isEmpty()) {
            map["x-kde-appname"] = config->appname;
    }

    // let's see if we've got an image, and store the image in the hints map
    if (!config->image.isNull()) {
        QImage image = config->image.toImage();
        map["image_data"] = ImageConverter::variantForImage(image);
    }

    args.append( map ); // hints
    args.append( config->timeout ); // expire timout

    m.setArguments( args );
    QDBusMessage replyMsg = QDBusConnection::sessionBus().call(m);

    delete config;

    if(replyMsg.type() == QDBusMessage::ReplyMessage) {
        if (!replyMsg.arguments().isEmpty()) {
            uint dbus_id = replyMsg.arguments().at(0).toUInt();
            if (dbus_id == 0)
            {
                kDebug() << "error: dbus_id is null";
                return false;
            }
            if (dbus_replaces_id && dbus_id == dbus_replaces_id)
                return true;
#if 1
            int oldId = m_idMap.key(dbus_id, 0);
            if (oldId != 0) {
                kWarning() << "Received twice the same id "<< dbus_id << "( previous notification: " << oldId << ")";
                m_idMap.remove(oldId);
                finish(oldId);
            }
#endif
            m_idMap.insert(id, dbus_id);
            kDebug() << "mapping knotify id to dbus id:"<< id << "=>" << dbus_id;

            return true;
        } else {
            kDebug() << "error: received reply with no arguments";
        }
    } else if (replyMsg.type() == QDBusMessage::ErrorMessage) {
        kDebug() << "error: failed to send dbus message";
    } else {
        kDebug() << "unexpected reply type";
    }
    return false;
}

void NotifyByPopup::closeNotificationDBus(int id)
{
    uint dbus_id = m_idMap.take(id);
    if (dbus_id == 0) {
        kDebug() << "not found dbus id to close" << id;
        return;
    }

    QDBusMessage m = QDBusMessage::createMethodCall( dbusServiceName, dbusPath,
            dbusInterfaceName, "CloseNotification" );
    QList<QVariant> args;
    args.append( dbus_id );
    m.setArguments( args );
    bool queued = QDBusConnection::sessionBus().send(m);
    if(!queued)
    {
        kDebug() << "warning: failed to queue dbus message";
    }

}

QStringList NotifyByPopup::popupServerCapabilities()
{
    if (!m_dbusServiceExists) {
        if( NotifyByPopupGrowl::canPopup() ) {
            return NotifyByPopupGrowl::capabilities();
        } else {
            // Return capabilities of the KPassivePopup implementation
            return QStringList() << "actions" << "body" << "body-hyperlinks"
                                 << "body-markup" << "icon-static";
        }
    }

    if(m_dbusServiceCapCacheDirty) {
        QDBusMessage m = QDBusMessage::createMethodCall( dbusServiceName, dbusPath,
                                  dbusInterfaceName, "GetCapabilities" );
        QDBusMessage replyMsg = QDBusConnection::sessionBus().call(m);
        if (replyMsg.type() != QDBusMessage::ReplyMessage) {
            kWarning(300) << "Error while calling popup server GetCapabilities()";
            return QStringList();
        }

        if (replyMsg.arguments().isEmpty()) {
            kWarning(300) << "popup server GetCapabilities() returned an empty reply";
            return QStringList();
        }

        m_dbusServiceCapabilities = replyMsg.arguments().at(0).toStringList();
        m_dbusServiceCapCacheDirty = false;
    }

    return m_dbusServiceCapabilities;
}


KNotifyConfig *NotifyByPopup::ensurePopupCompatibility( const KNotifyConfig *config )
{
    KNotifyConfig *c = config->copy();
    QStringList cap = popupServerCapabilities();

    if( !cap.contains( "actions" ) )
    {
        c->actions.clear();
    }

    if( !cap.contains( "body-markup" ) )
    {
        if( c->title.startsWith( "<html>" ) )
            c->title = stripHtml( config->title );
        if( c->text.startsWith( "<html>" ) )
            c->text  = stripHtml( config->text  );
    }

    return c;
}

QString NotifyByPopup::stripHtml( const QString &text )
{
    QXmlStreamReader r( "<elem>" + text + "</elem>" );
    HtmlEntityResolver resolver;
    r.setEntityResolver( &resolver );
    QString result;
    while( !r.atEnd() ) {
        r.readNext();
        if( r.tokenType() == QXmlStreamReader::Characters )
        {
            result.append( r.text() );
        }
        else if( r.tokenType() == QXmlStreamReader::StartElement
              && r.name()      == "br" )
        {
            result.append( "\n" );
        }
    }

    if(r.hasError())
    {
        // XML error in the given text, just return the original string
        kWarning(300) << "Notification to send to backend which does "
                         "not support HTML, contains invalid XML:"
                      << r.errorString() << "line" << r.lineNumber()
                      << "col" << r.columnNumber();
        return text;
    }

    return result;
}

QString NotifyByPopup::HtmlEntityResolver::resolveUndeclaredEntity(
        const QString &name )
{
    QString result =
        QXmlStreamEntityResolver::resolveUndeclaredEntity(name);
    if( !result.isEmpty() )
        return result;

    QChar ent = KCharsets::fromEntity( '&' + name );
    if( ent.isNull() ) {
        kWarning(300) << "Notification to send to backend which does "
                         "not support HTML, contains invalid entity: "
                      << name;
        ent = ' ';
    }
    return QString(ent);
}

#include "notifybypopup.moc"
