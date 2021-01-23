/*
    This file is part of the KDE Frameworks
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knotificationreplyaction.h"

#include <QString>

class KNotificationReplyActionPrivate
{
public:
    QString label;
    QString placeholderText;
    QString submitButtonText;
    QString submitButtonIconName;
    KNotificationReplyAction::FallbackBehavior fallbackBehavior = KNotificationReplyAction::FallbackBehavior::HideAction;
};

KNotificationReplyAction::KNotificationReplyAction(const QString &label)
    : QObject()
    , d(new KNotificationReplyActionPrivate)
{
    d->label = label;
}

KNotificationReplyAction::~KNotificationReplyAction()
{
    delete d;
}

QString KNotificationReplyAction::label() const
{
    return d->label;
}

void KNotificationReplyAction::setLabel(const QString &label)
{
    d->label = label;
}

QString KNotificationReplyAction::placeholderText() const
{
    return d->placeholderText;
}

void KNotificationReplyAction::setPlaceholderText(const QString &placeholderText)
{
    d->placeholderText = placeholderText;
}

QString KNotificationReplyAction::submitButtonText() const
{
    return d->submitButtonText;
}

void KNotificationReplyAction::setSubmitButtonText(const QString &submitButtonText)
{
    d->submitButtonText = submitButtonText;
}

QString KNotificationReplyAction::submitButtonIconName() const
{
    return d->submitButtonIconName;
}

void KNotificationReplyAction::setSubmitButtonIconName(const QString &submitButtonIconName)
{
    d->submitButtonIconName = submitButtonIconName;
}

KNotificationReplyAction::FallbackBehavior KNotificationReplyAction::fallbackBehavior() const
{
    return d->fallbackBehavior;
}

void KNotificationReplyAction::setFallbackBehavior(FallbackBehavior fallbackBehavior)
{
    d->fallbackBehavior = fallbackBehavior;
}
