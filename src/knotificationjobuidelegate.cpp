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

#include "knotificationjobuidelegate.h"

#include <QString>

#include <KJob>
#include <KNotification>

class KNotificationJobUiDelegatePrivate
{
public:
    void showNotification(KNotification::StandardEvent standardEvent, const QString &text);

    QString description;
};

void KNotificationJobUiDelegatePrivate::showNotification(KNotification::StandardEvent standardEvent, const QString &text)
{
    KNotification::event(standardEvent, description, text);
}

KNotificationJobUiDelegate::KNotificationJobUiDelegate()
    : KJobUiDelegate()
    , d(new KNotificationJobUiDelegatePrivate)
{
}

KNotificationJobUiDelegate::~KNotificationJobUiDelegate() = default;

bool KNotificationJobUiDelegate::setJob(KJob *job)
{
    const bool ok = KJobUiDelegate::setJob(job);

    if (ok) {
        connect(job, &KJob::description, this, [this](
                KJob *, const QString &title, const QPair<QString, QString> &, const QPair<QString, QString> &
        ){
            d->description = title;
        });
    }

    return ok;
}

void KNotificationJobUiDelegate::showErrorMessage()
{
    if (job()->error() == KJob::KilledJobError) {
        return;
    }

    d->showNotification(KNotification::Error, job()->errorString());
}

void KNotificationJobUiDelegate::slotWarning(KJob *job, const QString &plain, const QString &rich)
{
    Q_UNUSED(job);
    Q_UNUSED(rich);

    if (isAutoErrorHandlingEnabled()) {
        d->showNotification(KNotification::Notification, plain);
    }
}

#include "knotificationjobuidelegate.moc"
