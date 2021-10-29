/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDELIBS_KNOTIFICATIONRESTRICTIONS_H
#define KDELIBS_KNOTIFICATIONRESTRICTIONS_H

#include <knotifications_export.h>

#include <QObject>

/**
 * @class KNotificationRestrictions knotificationrestrictions.h KNotificationRestrictions
 *
 * KNotificationRestrictions provides a simple mechanism to avoid disruptions
 * during full screen presentations or other use cases where the screensaver or
 * desktop notifications are inappropriate.
 *
 * Using KNotificationRestrictions is quite straightforward: create an instance
 * of KNotificationRestrictions, passing in the set of or'd flags representing
 * the services that should be prevented from interrupting the user. When done
 * (for instance when the presentation is complete) simply delete the
 * KNotificationRestrictions object.
 *
 * Example: to ensure the screensaver does not turn on during a presentation:
 * @code
 * void MyApp::doPresentation()
 * {
 *   KNotificationRestrictions restrict(KNotificationRestrictions::ScreenSaver);
 *   // show presentation
 * }
 * @endcode
 */
class KNOTIFICATIONS_EXPORT KNotificationRestrictions : public QObject
{
    Q_OBJECT

public:
    /**
     * @enum Service
     * @see Services
     */
    enum Service {
        /**
         * The baseline "don't disable anything" value.
         */
        NoServices = 0,
        /**
         * Causes the screensaver to be prevented from automatically
         * turning on.
         */
        ScreenSaver = 1,
        /**
         * Causes instant messaging and email notifications to not appear.
         *
         * @note <b>not implemented yet</b>
         */
        MessagingPopups = 2,
        /**
         * Causes non-critical desktop messages to be suppressed.
         *
         * @note <b>not implemented yet</b>
         */
        Notifications = 4,
        /**
         * Causes all desktop notifications, including critical ones
         * (such as as "battery low" warnings) to be suppressed.
         *
         * @note <b>not implemented yet</b>
         */
        CriticalNotifications = 8,
        NonCriticalServices = ScreenSaver | MessagingPopups | Notifications,
        AllServices = NonCriticalServices | CriticalNotifications,
    };
    /**
     * Stores a combination of #Service values.
     */
    Q_DECLARE_FLAGS(Services, Service)

    /**
     * Constructs a new service for restrict some services.
     *
     * @param control the services to be restricted
     * @param parent the parent of this object
     */
    explicit KNotificationRestrictions(Services control = NonCriticalServices, QObject *parent = nullptr);
    ~KNotificationRestrictions() override;

    /**
     * Constructs a new service for restrict some services.
     *
     * @param control the services to be restricted
     * @param reason the reason for restriction
     * @param parent the parent of this object
     */
    // TODO KF6 make reason optional
    explicit KNotificationRestrictions(Services control, const QString &reason, QObject *parent = nullptr);

private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void screensaverFakeKeyEvent())
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KNotificationRestrictions::Services)
#endif
