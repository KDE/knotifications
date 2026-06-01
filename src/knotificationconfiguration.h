/*
    SPDX-FileCopyrightText: 2020-2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNOTIFICATIONCONFIGURATION_H
#define KNOTIFICATIONCONFIGURATION_H

#include <knotifications_export.h>

#include <qobjectdefs.h>

class KNotificationConfigurationPrivate;

/*!
 * \class KNotificationConfiguration
 * \inheaderfile KNotificationConfiguration
 * \inmodule KNotifications
 *
 * \brief Configure notifications using the means of the platform.
 *
 * This provides ways to interact with platform-specific means of
 * configuring notifications of the current application.
 *
 * This is particularly relevant for sandboxed applications (Flatpak, APK, etc)
 * where the host platform ultimately decides if/how notifications are shown.
 *
 * \since 6.28
 */
class KNOTIFICATIONS_EXPORT KNotificationConfiguration
{
    Q_GADGET
    /*!
     * \property KNotificationConfiguration::isAvailable
     */
    Q_PROPERTY(bool isAvailable READ isAvailable)

public:
    /*!
     * Returns \c true if notifications can be configured on the current platform.
     */
    [[nodiscard]] static bool isAvailable();

    /*!
     * Opens a platform-specific UI for configuring the notifications of the current
     * application.
     */
    Q_INVOKABLE static void show();

private:
    KNotificationConfigurationPrivate *d;
};

#endif
