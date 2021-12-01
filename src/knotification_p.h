/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2013-2014 Martin Klapetek <mklapetek@kde.org>

    code from KNotify/KNotifyClient
    SPDX-FileCopyrightText: 1997 Christian Esken <esken@kde.org>
    SPDX-FileCopyrightText: 2000 Charles Samuels <charles@kde.org>
    SPDX-FileCopyrightText: 2000 Stefan Schimanski <1Stein@gmx.de>
    SPDX-FileCopyrightText: 2000 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2000-2003 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2005 Allan Sandfeld Jensen <kde@carewolf.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KNOITIFICATION_P_H
#define KNOITIFICATION_P_H

#include <QTimer>

struct Q_DECL_HIDDEN KNotification::Private {
    QString eventId;
    int id = -1;
    int ref = 0;

    QWidget *widget = nullptr;
    QString title;
    QString text;
    QString iconName;
    QString defaultAction;
    QStringList actions;
    QString xdgActivationToken;
    std::unique_ptr<KNotificationReplyAction> replyAction;
    QPixmap pixmap;
    ContextList contexts;
    NotificationFlags flags = KNotification::CloseOnTimeout;
    QString componentName;
    KNotification::Urgency urgency = KNotification::DefaultUrgency;
    QVariantMap hints;

    QTimer updateTimer;
    bool needUpdate = false;
    bool isNew = true;
    bool autoDelete = true;

#if KNOTIFICATIONS_BUILD_DEPRECATED_SINCE(5, 67)
    /**
     * recursive function that raise the widget. @p w
     *
     * @see raiseWidget()
     */
    void raiseWidget(QWidget *w);
#endif
};

#endif
