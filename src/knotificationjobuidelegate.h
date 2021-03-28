/*
    This file is part of the KDE Frameworks
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNOTIFICATIONJOBUIDELEGATE_H
#define KNOTIFICATIONJOBUIDELEGATE_H

#include <KJobUiDelegate>

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
