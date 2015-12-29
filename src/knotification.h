/* This file is part of the KDE libraries
   Copyright (C) 2005-2006 Olivier Goffart <ogoffart at kde.org>
   Copyright (C) 2013-2015 Martin Klapetek <mklapetek@kde.org>

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

#ifndef KNOTIFICATION_H
#define KNOTIFICATION_H

#include <knotifications_export.h>

#include <QPixmap>
#include <QtCore/QObject>
#include <QtCore/QPair>

class QWidget;
class QDBusError;

/**
 * KNotification is used to notify the user of an event.
 *
 * \section introduction
 *
 * There are two main kinds of notifications:
 *
 * @li Feedback events:
 * For notifying the user that he/she just performed an operation, like maximizing a
 * window. This allows us to play sounds when a dialog appears.
 * This is an instant notification.  It ends automatically after a small timeout.
 *
 * @li persistant notifications:
 * Notify when the user received a new message, or when something else important happened
 * the user has to know about.  This notification has a start and a end.  It begins when
 * the event actually occurs, and finishes when the message is acknowledged or read.
 *
 * Example of a persistent notification in an instant messaging application:
 * The application emits the notification when the message is actually received, and closes it only
 * when the user has read the message (when the message window has received the focus) using the close() slot
 * Persistent notifications must have the Persistent flag.
 *
 * By default a notification will use the application name as title, but you
 * can also provide a brief text in the title and a more precise description in
 * the body text.  This is especially useful for notifications coming from
 * applications which should be considered "part of the system", like a battery
 * monitor or a network connection manager.
 * For example a battery indicator could use "Low Battery" as a title and "Only
 * 12 minutes left." as a body text.
 *
 * In order to perform a notification, you need to create a description file, which contains
 * default parameters of the notification, and use KNotification::event at the place in the
 * application code where the notification occurs.
 * The returned KNotification pointer may be used to connect signals or slots
 *
 * \section file The global config file
 * Your application should install a file called <em>knotifications5/appname.notifyrc</em>
 * in a QStandardPaths::GenericDataLocation directory.
 *
 * The filename should either match QCoreApplication::applicationName or be specified as the
 * component name to the KNotification object.
 *
 * You can do this with the following CMake command:
 * install(FILES appname.notifyrc  DESTINATION ${KNOTIFYRC_INSTALL_DIR}))
 *
 *  This file contains  mainly 3 parts
 *   <ol><li>\ref global "Global information"</li>
 *       <li>\ref context "Context information"</li>
 *       <li>\ref events "Definition of individual events"</li></ol>
 *
 *  \subsection global Global information
 * The global part looks like that
 * <pre>
           [Global]
           IconName=Filename
           Comment=Friendly Name of app
           Name=Name of app
 * </pre>
 *   The icon filename is just the name, without extension,  it's found with the KIconLoader.
 *   The Comment field will be used in KControl to describe the application.
 *   The Name field is optional and may be used as the application name for popup,
 *   if Name is not present, Comment is used instead
 *
 * \subsection context Context information
 *
 * This part consists of hints for the configuration widget
 *  <pre>
           [Context/group]
           Name=Group name
           Comment=The name of the group for contacts

           [Context/folder]
           Name=Group name
 *  </pre>
 *  The second part of the groupname is the context identifier.
 *  It should not contain special characters.
 *  The Name field is the one the user will see (and which is translated)
 *
 * \subsection events Definition of Events
 *
 * The definition of the events forms the most important part of the config file
 * <pre>
           [Event/newmail]
           Name=New email
           Comment=You have got a new email
           Contexts=folder,group
           Action=Sound|Popup

           [Event/contactOnline]
           Name=Contact goes online
           Comment=One of your contact has been connected
           Contexts=group
           Sound=filetoplay.ogg
           Action=None
 *  </pre>
 *  These are the default settings for each notifiable event.
 *  Action is the string representing the action. Actions can be added to
 *  KNotification as plugins, by deriving from KNotificationPlugin.
 *  At the time of writing, the following actions are available: Taskbar,
 *  Sound, Popup, Logfile, TTS, Execute.
 *  Actions can be combined by seperating them with '|'.
 *
 *  Contexts is a comma separated list of possible context for this event.
 *
 *  \section userfile The user's config file
 *
 *  This is an implementation detail, and is described here for your information.
 *
 *  In the config file, there are two parts:  the event configuration, and the context information
 * \subsection context Context information
 *  These are hints for the configuration dialog. They contain both the internal id of the context, and the user visible string.
 *  <pre>
           [Context/group]
           Values=1:Friends,2:Work,3:Family
 *  </pre>
 * \subsection event Events configuration
 *   This contains the configuration of events for the user.
 *   It contains the same fields as the description file.
 *    The key of groups is in the form
 *  <em>Event/&lt;EventName&gt;/&lt;ContextName&gt;/&lt;ContextValue&gt;</em>
 * <pre>
           [Event/contactOnline]
           Action=Sound
           Sound=/usr/share/sounds/super.ogg

           [Event/contactOnline/group/1]
           Action=Popup|Sound
 * </pre>
 *
 * \section example Example of code
 *
 * This portion of code will fire the event for the "contactOnline" event
 *
 * @code
    KNotification *notification= new KNotification ( "contactOnline", widget );
    notification->setText( i18n("The contact <i>%1</i> has gone online", contact->name() );
    notification->setPixmap( contact->pixmap() );
    notification->setActions( QStringList( i18n( "Open chat" ) ) );

    foreach( const QString &group , contact->groups() ) {
        notification->addContext( "group" , group ) ;
    }

    connect(notification, SIGNAL(activated(unsigned int )), contact , SLOT(slotOpenChat()) );

    notification->sendEvent();
 * @endcode
 *
 * @author Olivier Goffart  \<ogoffart at kde.org\>
 */
class KNOTIFICATIONS_EXPORT KNotification : public QObject
{
    Q_OBJECT

public:
    /**
     * Sometimes the user may want different notifications for the same event,
     * depending the source of the event.  Example, you want to be notified for mails
     * that arrive in your folder "personal inbox" but not for those in "spam" folder
     *
     * A notification context is a pair of two strings.
     * The first string is a key from what the context is.  example "group" or
     * "filter" (not translated).
     * The second is the id of the context. In our example, the group id or the
     * filter id in the applications.
     * These strings are the ones present in the config file, and are in theory not
     * shown in the user interface.
     *
     * The order of contexts in the list is is important, the most important context
     * should be placed first. They are processed in that order when the notification occurs.
     *
     * @see event
     */
    typedef QPair<QString, QString> Context;
    typedef QList< Context > ContextList;

    enum NotificationFlag {
        /**
         * When the notification is activated, raise the notification's widget.
         *
         * This will change the desktop, raise the window, and switch to the tab.
         * @todo  doesn't work yet
         */
        RaiseWidgetOnActivation = 0x01,

        /**
         * The notification will be automatically closed after a timeout. (this is the default)
         */
        CloseOnTimeout = 0x00,

        /**
         * The notification will NOT be automatically closed after a timeout.
         * You will have to track the notification, and close it with the
         * close function manually when the event is done, otherwise there will be a memory leak
         */
        Persistent = 0x02,

        /**
         * The notification will be automatically closed if the widget() becomes
         * activated.
         *
         * If the widget is already activated when the notification occurs, the
         * notification will be closed after a small timeout.
         *
         * This only works if the widget is the toplevel widget
         * @todo make it work with tabulated widget
         */
        CloseWhenWidgetActivated = 0x04,

        /**
         * The audio plugin will loop the sound until the notification is closed
         */
        LoopSound = 0x08,

        /**
         * Sends a hint to Plasma to skip grouping for this notification
         *
         * @since: 5.18
         */
        SkipGrouping = 0x10,

        /**
         * @internal
         * The event is a standard kde event, and not an event of the application
         */
        DefaultEvent = 0xF000

    };

    Q_DECLARE_FLAGS(NotificationFlags, NotificationFlag)

    /**
     * default events you can use in the event function
     */
    enum StandardEvent { Notification, Warning, Error, Catastrophe };

    /**
     * Create a new notification.
     *
     * You have to use sendEvent to show the notification.
     *
     * The pointer is automatically deleted when the event is closed.
     *
     * Make sure you use one of the NotificationFlags CloseOnTimeOut or
     * CloseWhenWidgetActivated, if not,
     * you have to close the notification yourself.
     *
     * @param eventId is the name of the event
     * @param widget is a widget where the notification reports to
     * @param flags is a bitmask of NotificationFlag
     */
    explicit KNotification(const QString &eventId, QWidget *widget = 0L, const NotificationFlags &flags = CloseOnTimeout);

    /**
     * Create a new notification.
     *
     * You have to use sendEvent to show the notification.
     *
     * The pointer is automatically deleted when the event is closed.
     *
     * Make sure you use one of the NotificationFlags CloseOnTimeOut or
     * CloseWhenWidgetActivated, if not,
     * you have to close the notification yourself.
     *
     * @since 4.4
     *
     * @param eventId is the name of the event
     * @param flags is a bitmask of NotificationFlag
     * @param parent parent object
     */
    // KDE5: Clean up this mess
    // Only this constructor should stay with saner argument order and
    // defaults. Because of binary and source compatibility issues it has to
    // stay this way for now. The second argument CANNOT have a default
    // argument. if someone needs a widget associated with the notification he
    // should use setWidget after creating the object (or some xyz_cast magic)
    explicit KNotification(const QString &eventId, const NotificationFlags &flags, QObject *parent = NULL);

    ~KNotification();

    /**
     * @brief the widget associated to the notification
     *
     * If the widget is destroyed, the notification will be automatically cancelled.
     * If the widget is activated, the notification will be automatically closed if the NotificationFlags specify that
     *
     * When the notification is activated, the widget might be raised.
     * Depending on the configuration, the taskbar entry of the window containing the widget may blink.
     */
    QWidget *widget() const;

    /**
     * Set the widget associated to the notification.
     * The notification is reparented to the new widget.
     * \see widget()
     * @param widget the new widget
     */
    void setWidget(QWidget *widget);

    /**
     * @return the name of the event
     */
    QString eventId() const;

    /**
     * @return the notification title
     * @see setTitle
     * @since 4.3
     */
    QString title() const;

    /**
     * Set the title of the notification popup.
     * If no title is set, the application name will be used.
     *
     * @param title The title of the notification
     * @since 4.3
     */
    void setTitle(const QString &title);

    /**
     * @return the notification text
     * @see setText
     */
    QString text() const;

    /**
     * Set the notification text that will appear in the popup.
     *
     * In Plasma workspace, the text is shown in a QML label which uses Text.StyledText,
     * ie. it supports a small subset of HTML entities (mostly just formatting tags)
     *
     * If the notifications server does not advertise "body-markup" capability,
     * all HTML tags are stripped before sending it to the server
     *
     * @param text The text to display in the notification popup
     */
    void setText(const QString &text);

    /**
     * \return the icon shown in the popup
     * \see setIconName
     * \since 5.4
     */
    QString iconName() const;

    /**
     * Set the icon that will be shown in the popup.
     *
     * @param icon the icon
     * @since 5.4
     */
    void setIconName(const QString &icon);

    /**
     * \return the pixmap shown in the popup
     * \see setPixmap
     */
    QPixmap pixmap() const;
    /**
     * Set the pixmap that will be shown in the popup.
     *
     * @param pix the pixmap
     */
    void setPixmap(const QPixmap &pix);

    /**
     * @return the list of actions
     */
    QStringList actions() const;

    /**
     * Set the list of actions shown in the popup. The strings passed
     * in that QStringList will be used as labels for those actions,
     * so ideally they should be wrapped in i18n() or tr() calls.
     * In Plasma workspace, these will be shown as buttons inside
     * the notification popup.
     *
     * The visual representation of actions however depends
     * on the notification server
     *
     * @param actions List of strings used as action labels
     */
    void setActions(const QStringList &actions);

    /**
     * @return the list of contexts, see KNotification::Context
     */
    ContextList contexts() const;
    /**
     * set the list of contexts, see KNotification::Context
     *
     * The list of contexts must be set before calling sendEvent;
     */
    void setContexts(const ContextList &contexts);
    /**
     * append a context at the list of contexts, see KNotificaiton::Context
     * @param context the context which is added
     */
    void addContext(const Context &context);
    /**
     * @overload
     * @param context_key is the key of the context
     * @param context_value is the value of the context
     */
    void addContext(const QString &context_key, const QString &context_value);

    /**
     * @return the notification flags.
     */
    NotificationFlags flags() const;

    /**
     * Set the notification flags.
     * These must be set before calling sendEvent()
     */
    void setFlags(const NotificationFlags &flags);

    /**
     * The componentData is used to determine the location of the config file.
     *
     * If no componentName is set, the app name is used by default
     *
     * @param componentName the new component name
     */
    void setComponentName(const QString &componentName);

    /**
     * @internal
     * the id given by the notification manager
     */
    int id();

    /**
     * @internal
     * appname used for the dbus object
     */
    QString appName() const;

Q_SIGNALS:
    /**
     * Emitted only when the default activation has occurred
     */
    void activated();
    /**
     * Emitted when an action has been activated.
     *
     * The parameter passed by the signal is the index of the action
     * in the QStringList set by setActions() call.
     *
     * @param action will be 0 is the default aciton was activated, or the index of the action in the actions QStringList
     */
    void activated(unsigned int action);

    /**
     * Convenience signal that is emitted when the first action is activated.
     */
    void action1Activated();

    /**
     * \overload
     */
    void action2Activated();

    /**
     * \overload
     */
    void action3Activated();

    /**
     * Emitted when the notification is closed.
     *
     * Can be closed either by the user clicking the close button,
     * the timeout running out or when an action was triggered.
     */
    void closed();

    /**
     * The notification has been ignored
     */
    void ignored();

public Q_SLOTS:
    /**
     * @brief Activate the action specified action
     * If the action is zero, then the default action is activated
     */
    void activate(unsigned int action = 0);

    /**
     * Close the notification without activating it.
     *
     * This will delete the notification.
     */
    void close();

    /**
     * @brief Raise the widget.
     * This will change the desktop, activate the window, and the tab if needed.
     */
    void raiseWidget();

    /**
     * The notification will automatically be closed if all presentations are finished.
     * if you want to show your own presentation in your application, you should use this
     * function, so it will not be automatically closed when there is nothing to show.
     *
     * Don't forgot to deref, or the notification may be never closed if there is no timeout.
     *
     * @see deref
     */
    void ref();
    /**
     * Remove a reference made with ref(). If the ref counter hits zero,
     * the notification will be closed and deleted.
     *
     * @see ref
     */
    void deref();

    /**
     * Send the notification to the server.
     *
     * This will cause all the configured plugins to execute their actions on this notification
     * (eg. a sound will play, a popup will show, a command will be executed etc).
     */
    void sendEvent();

    /**
     * @internal
     * update the texts, the icon, and the actions of one existing notification
     */
    void update();

private:
    struct Private;
    Private *const d;

protected:
    /**
     * reimplemented for internal reasons
     */
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
    static QString standardEventToEventId(StandardEvent event);
    static QString standardEventToIconName(StandardEvent event);

public:
    /**
     * @brief emit an event
     *
     * This method creates the KNotification, setting every parameter, and fire the event.
     * You don't need to call sendEvent
     *
     * A popup may be displayed or a sound may be played, depending the config.
     *
     * @return a KNotification .  You may use that pointer to connect some signals or slot.
     * the pointer is automatically deleted when the event is closed.
     *
     * Make sure you use one of the CloseOnTimeOut or CloseWhenWidgetActivated, if not,
     * you have to close yourself the notification.
     *
     * @note the text is shown in a QLabel, you should escape HTML, if needed.
     *
     * @param eventId is the name of the event
     * @param title is title of the notification to show in the popup.
     * @param text is the text of the notification to show in the popup.
     * @param pixmap is a picture which may be shown in the popup.
     * @param widget is a widget where the notification reports to
     * @param flags is a bitmask of NotificationFlag
     * @param componentName used to determine the location of the config file.  by default, appname is used
     * @since 4.4
     */
    static KNotification *event(const QString &eventId, const QString &title, const QString &text,
                                const QPixmap &pixmap = QPixmap(), QWidget *widget = 0L,
                                const NotificationFlags &flags = CloseOnTimeout,
                                const QString &componentName = QString());

    /**
     * @brief emit a standard event
     *
     * @overload
     *
     * This will emit a standard event
     *
     * @param eventId is the name of the event
     * @param text is the text of the notification to show in the popup.
     * @param pixmap is a picture which may be shown in the popup.
     * @param widget is a widget where the notification reports to
     * @param flags is a bitmask of NotificationFlag
     * @param componentName used to determine the location of the config file.  by default, plasma_workspace is used
     */
    static KNotification *event(const QString &eventId, const QString &text = QString(),
                                const QPixmap &pixmap = QPixmap(), QWidget *widget = 0L,
                                const NotificationFlags &flags = CloseOnTimeout,
                                const QString &componentName = QString());

    /**
     * @brief emit a standard event
     *
     * @overload
     *
     * This will emit a standard event
     *
     * @param eventId is the name of the event
     * @param text is the text of the notification to show in the popup
     * @param pixmap is a picture which may be shown in the popup
     * @param widget is a widget where the notification reports to
     * @param flags is a bitmask of NotificationFlag
     */
    static KNotification *event(StandardEvent eventId, const QString &text = QString(),
                                const QPixmap &pixmap = QPixmap(), QWidget *widget = 0L,
                                const NotificationFlags &flags = CloseOnTimeout);

    /**
     * @brief emit a standard event
     *
     * @overload
     *
     * This will emit a standard event
     *
     * @param eventId is the name of the event
     * @param title is title of the notification to show in the popup.
     * @param text is the text of the notification to show in the popup
     * @param pixmap is a picture which may be shown in the popup
     * @param widget is a widget where the notification reports to
     * @param flags is a bitmask of NotificationFlag
     * @since 4.4
     */
    static KNotification *event(StandardEvent eventId, const QString &title, const QString &text,
                                const QPixmap &pixmap, QWidget *widget = 0L,
                                const NotificationFlags &flags = CloseOnTimeout);

    /**
     * @brief emit a standard event with the possibility of setting an icon by icon name
     *
     * @overload
     *
     * This will emit a standard event
     *
     * @param eventId is the name of the event
     * @param title is title of the notification to show in the popup.
     * @param text is the text of the notification to show in the popup
     * @param iconName a Freedesktop compatible icon name to be shown in the popup
     * @param widget is a widget where the notification reports to
     * @param flags is a bitmask of NotificationFlag
     * @param componentName used to determine the location of the config file.  by default, plasma_workspace is used
     * @since 5.4
     */
    static KNotification *event(const QString &eventId, const QString &title, const QString &text,
                                const QString &iconName, QWidget *widget = 0L,
                                const NotificationFlags &flags = CloseOnTimeout,
                                const QString &componentName = QString());

    /**
     * @brief emit a standard event with the possibility of setting an icon by icon name
     *
     * @overload
     *
     * This will emit a standard event with a custom icon
     *
     * @param eventId the type of the standard (not app-defined) event
     * @param title is title of the notification to show in the popup.
     * @param text is the text of the notification to show in the popup
     * @param iconName a Freedesktop compatible icon name to be shown in the popup
     * @param widget is a widget where the notification reports to
     * @param flags is a bitmask of NotificationFlag
     * @since 5.9
     */
    static KNotification *event(StandardEvent eventId, const QString &title, const QString &text,
                                const QString &iconName, QWidget *widget = 0L,
                                const NotificationFlags &flags = CloseOnTimeout);

    /**
     * @brief emit a standard event
     *
     * @overload
     *
     * This will emit a standard event with its standard icon
     *
     * @param eventId the type of the standard (not app-defined) event
     * @param title is title of the notification to show in the popup.
     * @param text is the text of the notification to show in the popup
     * @param widget is a widget where the notification reports to
     * @param flags is a bitmask of NotificationFlag
     * @since 5.9
     */
    static KNotification *event(StandardEvent eventId, const QString &title, const QString &text,
                                QWidget *widget = 0L, const NotificationFlags &flags = CloseOnTimeout);


    /**
     * This is a simple substitution for QApplication::beep()
     *
     * @param reason a short text explaining what has happened (may be empty)
     * @param widget the widget the notification refers to
     */
    static void beep(const QString &reason = QString(), QWidget *widget = 0L);

    //prevent warning
    using QObject::event;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KNotification::NotificationFlags)

#endif
