/*
    This file is part of the KDE Frameworks
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNOTIFICATIONREPLYACTION_H
#define KNOTIFICATIONREPLYACTION_H

#include <knotifications_export.h>

#include <QObject>

class QString;

class KNotificationReplyActionPrivate;

/**
 * @class KNotificationReplyAction knotificationreplyaction.h KNotificationReplyAction
 *
 * @brief An inline reply action
 *
 * This class represents an inline reply action, which lets the user type a
 * reply to a chat message or email in the notification popup.
 */
class KNOTIFICATIONS_EXPORT KNotificationReplyAction : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a inline reply action with given label
     * @param label The label for the action
     */
    explicit KNotificationReplyAction(const QString &label);
    /**
     * Destroys this inline reply action
     */
    ~KNotificationReplyAction() override;

    /**
     * The label for the action button
     */
    QString label() const;
    /**
     * Set the label for the action button
     */
    void setLabel(const QString &label);

    /**
     * The placeholder text for the inline reply text field
     */
    QString placeholderText() const;
    /**
     * Set the placeholder text for the inline reply text field, for example "Reply to Konqi..."
     */
    void setPlaceholderText(const QString &placeholderText);

    /**
     * The label for the button to send the typed reply
     */
    QString submitButtonText() const;
    /**
     * Set the label for the button to send the typed reply
     */
    void setSubmitButtonText(const QString &submitButtonText);

    /**
     * The icon name for the button to send the typed reply
     */
    QString submitButtonIconName() const;
    /**
     * Set the icon name for the button to send the typed reply
     */
    void setSubmitButtonIconName(const QString &submitButtonIconName);

    /**
     * Behavior when the notification server does not support inline replies
     */
    enum class FallbackBehavior {
        /**
         * Don't add the reply action (default)
         */
        HideAction,
        /**
         * Add the reply action as regular button
         *
         * Use this if you want to provide your own reply functionality
         *
         * @note The @c activated signal is emitted instead of @c replied!
         */
        UseRegularAction,
    };
    /**
     * Gets the fallback behavior when the notification server does not support inline replies
     */
    FallbackBehavior fallbackBehavior() const;
    /**
     * Set the fallback behavior for when the notification server does not support inline replies
     */
    void setFallbackBehavior(FallbackBehavior fallbackBehavior);

Q_SIGNALS:
    /**
     * Emitted when the user has submitted a reply
     *
     * @note This is never emitted when the notification server does not support inline replies
     *
     * @param text The text the user entered
     */
    void replied(const QString &text);
    /**
     * Emitted when the user clicks the reply fallback button
     *
     * @note This is emitted when the notification server does not support inline replies
     * and fallbackBehavior is set to @c UseRegularAction.
     */
    void activated();

private:
    KNotificationReplyActionPrivate *const d;
};

#endif // KNOTIFICATIONREPLYACTION_H
