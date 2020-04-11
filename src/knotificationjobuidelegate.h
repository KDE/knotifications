/* This file is part of the KDE Frameworks
   Copyright (C) 2020 Kai Uwe Broulik <kde@broulik.de>

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

#ifndef KNOTIFICATIONJOBUIDELEGATE_H
#define KNOTIFICATIONJOBUIDELEGATE_H

#include <kjobuidelegate.h>

#include <QScopedPointer>

#include <knotifications_export.h>

class KNotificationJobUiDelegatePrivate;

/**
 * @class KNotificationJobUiDelegate knotificationjobuidelegate.h KNotificationJobUiDelegate
 *
 * A UI delegate using KNotification for interaction (showing errors and warnings).
 *
 * @since 5.69
 */
class KNOTIFICATIONS_EXPORT KNotificationJobUiDelegate : public KJobUiDelegate
{
    Q_OBJECT

public:
    /**
     * Constructs a new KNotificationJobUiDelegate.
     */
    KNotificationJobUiDelegate();

    /**
     * Constructs a new KNotificationJobUiDelegate.
     * @param flags allows to enable automatic error/warning handling
     * @since 5.70
     */
    explicit KNotificationJobUiDelegate(KJobUiDelegate::Flags flags); // KF6 TODO merge with default constructor, using AutoHandlingDisabled as default value

    /**
     * Destroys the KNotificationJobUiDelegate.
     */
    ~KNotificationJobUiDelegate() override;

public:
    /**
     * Display a notification to inform the user of the error given by
     * this job.
     */
    void showErrorMessage() override;

protected Q_SLOTS:
    bool setJob(KJob *job) override;
    void slotWarning(KJob *job, const QString &plain, const QString &rich) override;

private:
    QScopedPointer<KNotificationJobUiDelegatePrivate> d;
};

#endif // KNOTIFICATIONJOBUIDELEGATE_H
