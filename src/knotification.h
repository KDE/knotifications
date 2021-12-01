/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2013-2015 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNOTIFICATION_H
#define KNOTIFICATION_H

#include <knotifications_export.h>

#include <QList>
#include <QObject>
#include <QPair>
#include <QPixmap>
#include <QUrl>
#include <QVariant>

#include <memory>

class QWidget;

class KNotificationReplyAction;

/**
 * @class KNotification knotification.h KNotification
 *
 * KNotification is the main class for creating notifications.
 */
class KNOTIFICATIONS_EXPORT KNotification : public QObject
{
    Q_OBJECT
    /**
     * @copydoc setEventId
     * @since 5.88
     */
    Q_PROPERTY(QString eventId READ eventId WRITE setEventId NOTIFY eventIdChanged)
    /**
     * @copydoc setTitle
     * @since 5.88
     */
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    /**
     * @copydoc setText
     * @since 5.88
     */
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    /**
     * @copydoc setIconName
     * @since 5.88
     */
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)
    /**
     * @copydoc setDefaultAction
     * @since 5.88
     */
    Q_PROPERTY(QString defaultAction READ defaultAction WRITE setDefaultAction NOTIFY defaultActionChanged)
    /**
     * @copydoc setActions
     * @since 5.88
     */
    Q_PROPERTY(QStringList actions READ actions WRITE setActions NOTIFY actionsChanged)
    /**
     * @copydoc setFlags
     * @since 5.88
     */
    Q_PROPERTY(NotificationFlags flags READ flags WRITE setFlags NOTIFY flagsChanged)
    /**
     * @copydoc setComponentName
     * @since 5.88
     */
    Q_PROPERTY(QString componentName READ componentName WRITE setComponentName NOTIFY componentNameChanged)
    /**
     * @copydoc setUrls
     * @since 5.88
     */
    Q_PROPERTY(QList<QUrl> urls READ urls WRITE setUrls NOTIFY urlsChanged)
    /**
     * @copydoc setUrgency
     * @since 5.88
     */
    Q_PROPERTY(Urgency urgency READ urgency WRITE setUrgency NOTIFY urgencyChanged)
    /**
     * @copydoc setAutoDelete
     * @seince 5.88
     */
    Q_PROPERTY(bool autoDelete READ isAutoDelete WRITE setAutoDelete NOTIFY autoDeleteChanged)
    /**
     * @since 5.90
     */
    Q_PROPERTY(QString xdgActivationToken READ xdgActivationToken NOTIFY xdgActivationTokenChanged)

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
    typedef QList<Context> ContextList;

    /**
     * @see NotificationFlags
     */
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
         * @since 5.18
         */
        SkipGrouping = 0x10,

        /**
         * @internal
         * The event is a standard kde event, and not an event of the application
         */
        DefaultEvent = 0xF000,

    };

    /**
     * Stores a combination of #NotificationFlag values.
     */
    Q_DECLARE_FLAGS(NotificationFlags, NotificationFlag)
    Q_FLAG(NotificationFlags)

    /**
     * default events you can use in the event function
     */
    enum StandardEvent {
        Notification,
        Warning,
        Error,
        Catastrophe,
    };

    /**
     * The urgency of a notification.
     *
     * @since 5.58
     * @sa setUrgency
     */
    enum Urgency {
        DefaultUrgency = -1,
        LowUrgency = 10,
        NormalUrgency = 50,
        HighUrgency = 70,
        CriticalUrgency = 90,
    };
    Q_ENUM(Urgency)

#if KNOTIFICATIONS_ENABLE_DEPRECATED_SINCE(5, 75)
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
     * @deprecated Since 5.75, use other constructor and call setWidget() explicitly
     */
    KNOTIFICATIONS_DEPRECATED_VERSION(5, 75, "Use other constructor and call setWidget() explicitly")
    explicit KNotification(const QString &eventId, QWidget *widget, const NotificationFlags &flags = CloseOnTimeout);
#endif

    /**
     * Create a new notification.
     *
     * You have to use sendEvent to show the notification.
     *
     * The pointer is automatically deleted when the event is closed.
     *
     * The NotificationFlags is set to CloseOnTimeout.
     *
     * @param eventId is the name of the event
     * @since 5.75
     */
    inline explicit KNotification(const QString &eventId)
        : KNotification(eventId, CloseOnTimeout, nullptr)
    {
    }
    // TODO KF6: remove thic constructor, in favour of other non-deprecated constructor
    // It is a helper constructor for software binary compatibility of code which called the
    // deprecated constructor so far due to it's second parameter "widget" having had
    // a default nullptr value.

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
    explicit KNotification(const QString &eventId, const NotificationFlags &flags, QObject *parent = nullptr);
    // TODO KF6: pass flags by value instead of reference, set CloseOnTimeout as default value

    ~KNotification() override;

    /**
     * @brief the widget associated to the notification
     *
     * If the widget is destroyed, the notification will be automatically canceled.
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
     * Set the event id, if not already passed to the constructor.
     * @since 5.88
     */
    void setEventId(const QString &eventId);

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
     * Set the pixmap that will be shown in the popup. If you want to use an icon from the icon theme use setIconName instead.
     *
     * @param pix the pixmap
     */
    void setPixmap(const QPixmap &pix);

    /**
     * @return the default action, or an empty string if not set
     * @since 5.31
     */
    QString defaultAction() const;

    /**
     * Set a default action that will be triggered when the notification is
     * activated (typically, by clicking on the notification popup). The default
     * action should raise a window belonging to the application that sent it.
     *
     * The string will be used as a label for the action, so ideally it should
     * be wrapped in i18n() or tr() calls.
     *
     * The visual representation of actions depends on the notification server.
     * In Plasma and Gnome desktops, the actions are performed by clicking on
     * the notification popup, and the label is not presented to the user.
     *
     *
     * @param action Label of the default action. The label might or might not
     * be displayed to the user by the notification server, depending on the
     * implementation. Passing an empty string disables the default action.
     * @since 5.31
     */
    void setDefaultAction(const QString &defaultAction);

    /**
     * @return the list of actions
     */
    // KF6: Rename to "additionalActions"?
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
    // KF6: Rename to "setAdditionalActions"?
    void setActions(const QStringList &actions);

    /**
     * @return the inline reply action.
     * @since 5.81
     */
    KNotificationReplyAction *replyAction() const;

    /**
     * @brief Add an inline reply action to the notification.
     *
     * On supported platforms this lets the user type a reply to a notification,
     * such as replying to a chat message, from the notification popup, for example:
     *
     * @code{.cpp}
     * KNotification *notification = new KNotification(QStringLiteral("notification"));
     * ...
     * auto replyAction = std::make_unique<KNotificationReplyAction>(i18nc("@action:button", "Reply"));
     * replyAction->setPlaceholderText(i18nc("@info:placeholder", "Reply to Dave..."));
     * QObject::connect(replyAction.get(), &KNotificationReplyAction::replied, [](const QString &text) {
     *     qDebug() << "you replied with" << text;
     * });
     * notification->setReplyAction(std::move(replyAction));
     * @endcode
     *
     * @param replyAction the reply action to set
     * @since 5.81
     */
    void setReplyAction(std::unique_ptr<KNotificationReplyAction> replyAction);

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
     * Returns the component name used to determine the location of the configuration file.
     * @since 5.88
     */
    QString componentName() const;
    /**
     * The componentData is used to determine the location of the config file.
     *
     * If no componentName is set, the app name is used by default
     *
     * @param componentName the new component name
     */
    void setComponentName(const QString &componentName);

    /**
     * URLs associated with this notification
     * @since 5.29
     */
    QList<QUrl> urls() const;

    /**
     * Sets URLs associated with this notification
     *
     * For example, a screenshot application might want to provide the
     * URL to the file that was just taken so the notification service
     * can show a preview.
     *
     * @note This feature might not be supported by the user's notification service
     *
     * @param urls A list of URLs
     * @since 5.29
     */
    void setUrls(const QList<QUrl> &urls);

    /**
     * The urgency of the notification.
     * @since 5.58
     */
    Urgency urgency() const;

    /**
     * Sets the urgency of the notification.
     *
     * This defines the importance of the notification. For example,
     * a track change in a media player would be a low urgency.
     * "You have new mail" would be normal urgency. "Your battery level
     * is low" would be a critical urgency.
     *
     * Use critical notifications with care as they might be shown even
     * when giving a presentation or when notifications are turned off.
     *
     * @param urgency The urgency.
     * @since 5.58
     */
    void setUrgency(Urgency urgency);

    /**
     * @internal
     * the id given by the notification manager
     */
    int id();

    /**
     * @internal
     * appname used for the D-Bus object
     */
    QString appName() const;

    /**
     * Returns whether this notification object will be automatically deleted after closing.
     * @since 5.88
     */
    bool isAutoDelete() const;
    /**
     * Sets whether this notification object will be automatically deleted after closing.
     * This is on by default for C++, and off by default for QML.
     * @since 5.88
     */
    void setAutoDelete(bool autoDelete);

    /**
     * Returns the activation token to use to activate a window.
     * @since 5.90
     */
    QString xdgActivationToken() const;

Q_SIGNALS:
#if KNOTIFICATIONS_ENABLE_DEPRECATED_SINCE(5, 76)
    /**
     * Emitted only when the default activation has occurred
     * @deprecated Since 5.67, use defaultActivated() instead
     */
    KNOTIFICATIONS_DEPRECATED_VERSION_BELATED(5, 76, 5, 67, "Use defaultActivated() instead")
    void activated(); // clazy:exclude=overloaded-signal
#endif

    /**
     * Emitted when the default action has been activated.
     * @since 5.67
     */
    void defaultActivated();

    /**
     * Emitted when an action has been activated.
     *
     * The parameter passed by the signal is the index of the action
     * in the QStringList set by setActions() call.
     *
     * @param action will be 0 if the default action was activated, or the index of the action in the actions QStringList
     */
    void activated(unsigned int action); // clazy:exclude=overloaded-signal

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

    /**
     * Emitted when @c eventId changed.
     * @since 5.88
     */
    void eventIdChanged();
    /**
     * Emitted when @c title changed.
     * @since 5.88
     */
    void titleChanged();
    /**
     * Emitted when @c text changed.
     * @since 5.88
     */
    void textChanged();
    /**
     * Emitted when @c iconName changed.
     * @since 5.88
     */
    void iconNameChanged();
    /**
     * Emitted when @c defaultAction changed.
     * @since 5.88
     */
    void defaultActionChanged();
    /**
     * Emitted when @c actions changed.
     * @since 5.88
     */
    void actionsChanged();
    /**
     * Emitted when @p flags changed.
     * @since 5.88
     */
    void flagsChanged();
    /**
     * Emitted when @p componentName changed.
     * @since 5.88
     */
    void componentNameChanged();
    /**
     * Emitted when @p urls changed.
     * @since 5.88
     */
    void urlsChanged();
    /**
     * Emitted when @p urgency changed.
     * @since 5.88
     */
    void urgencyChanged();
    /**
     * Emitted when @p autoDelete changed.
     * @since 5.88
     */
    void autoDeleteChanged();
    /**
     * Emitted when @p xdgActivationToken changes.
     * @since 5.90
     */
    void xdgActivationTokenChanged();

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

#if KNOTIFICATIONS_ENABLE_DEPRECATED_SINCE(5, 67)
    /**
     * @brief Raise the widget.
     * This will change the desktop, activate the window, and the tab if needed.
     * @deprecated since 5.67, use QWindow raise + requestActivate instead.
     */
    KNOTIFICATIONS_DEPRECATED_VERSION(5, 67, "Use QWindow raise + requestActivate instead")
    void raiseWidget();
#endif

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

    /**
     * @since 5.57
     * Adds a custom hint to the notification. Those are key-value pairs that can be interpreted by the respective notification backend to trigger additional,
     * non-standard features.
     * @param hint the hint's key
     * @param value the hint's value
     */
    void setHint(const QString &hint, const QVariant &value);

    /**
     * @since 5.57
     * Returns the custom hints set by setHint()
     */
    QVariantMap hints() const;

private:
    friend class KNotificationManager;
    struct Private;
    Private *const d;

protected:
    /**
     * reimplemented for internal reasons
     */
    bool eventFilter(QObject *watched, QEvent *event) override;
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
     * @param eventId is the name of the event
     * @param title is title of the notification to show in the popup.
     * @param text is the text of the notification to show in the popup.
     * @param pixmap is a picture which may be shown in the popup.
     * @param widget is a widget where the notification reports to
     * @param flags is a bitmask of NotificationFlag
     * @param componentName used to determine the location of the config file.  by default, appname is used
     * @since 4.4
     */
    static KNotification *event(const QString &eventId,
                                const QString &title,
                                const QString &text,
                                const QPixmap &pixmap = QPixmap(),
                                QWidget *widget = nullptr,
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
    static KNotification *event(const QString &eventId,
                                const QString &text = QString(),
                                const QPixmap &pixmap = QPixmap(),
                                QWidget *widget = nullptr,
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
    static KNotification *event(StandardEvent eventId,
                                const QString &text = QString(),
                                const QPixmap &pixmap = QPixmap(),
                                QWidget *widget = nullptr,
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
    static KNotification *event(StandardEvent eventId,
                                const QString &title,
                                const QString &text,
                                const QPixmap &pixmap,
                                QWidget *widget = nullptr,
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
    static KNotification *event(const QString &eventId,
                                const QString &title,
                                const QString &text,
                                const QString &iconName,
                                QWidget *widget = nullptr,
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
    static KNotification *event(StandardEvent eventId,
                                const QString &title,
                                const QString &text,
                                const QString &iconName,
                                QWidget *widget = nullptr,
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
    static KNotification *
    event(StandardEvent eventId, const QString &title, const QString &text, QWidget *widget = nullptr, const NotificationFlags &flags = CloseOnTimeout);

    /**
     * This is a simple substitution for QApplication::beep()
     *
     * @param reason a short text explaining what has happened (may be empty)
     * @param widget the widget the notification refers to
     */
    static void beep(const QString &reason = QString(), QWidget *widget = nullptr);

    // prevent warning
    using QObject::event;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KNotification::NotificationFlags)

#endif
