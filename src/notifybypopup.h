/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2008 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NOTIFYBYPOPUP_H
#define NOTIFYBYPOPUP_H

#include "knotificationplugin.h"

#include <QStringList>

class KNotification;
class QDBusPendingCallWatcher;
class NotifyByPopupPrivate;

class NotifyByPopup : public KNotificationPlugin
{
    Q_OBJECT
public:
    explicit NotifyByPopup(QObject *parent = nullptr);
    ~NotifyByPopup() override;

    QString optionName() override { return QStringLiteral("Popup"); }
    void notify(KNotification *notification, KNotifyConfig *notifyConfig) override;
    void close(KNotification *notification) override;
    void update(KNotification *notification, KNotifyConfig *config) override;

private Q_SLOTS:
    // slot which gets called when DBus signals that some notification action was invoked
    void onNotificationActionInvoked(uint notificationId, const QString &actionKey);
    // slot which gets called when DBus signals that some notification was closed
    void onNotificationClosed(uint, uint);

private:
    // TODO KF6, replace current public notify/update
    void notify(KNotification *notification, const KNotifyConfig &notifyConfig);
    void update(KNotification *notification, const KNotifyConfig &notifyConfig);

    NotifyByPopupPrivate * const d;
    friend class NotifyByPopupPrivate;
};

#endif
