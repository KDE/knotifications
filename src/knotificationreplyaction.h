/*
    This file is part of the KDE Frameworks
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNOTIFICATIONREPLYACTION_H
#define KNOTIFICATIONREPLYACTION_H

#include <knotifications_export.h>

#include <QObject>

#include <memory>

class QString;

class KNotificationReplyActionPrivate;

/*!
 * \class KNotificationReplyAction
 * \inmodule KNotifications
 *
 * \brief An inline reply action.
 *
 * This class represents an inline reply action, which lets the user type a
 * reply to a chat message or email in the notification popup.
 */
class KNOTIFICATIONS_EXPORT KNotificationReplyAction : public QObject
{
    Q_OBJECT
    /*!
     * \property KNotificationReplyAction::label
     * \since 5.88
     */
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)
    /*!
     * \property KNotificationReplyAction::placeholderText
     * \since 5.88
     */
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
    /*!
     * \property KNotificationReplyAction::submitButtonText
     * \since 5.88
     */
    Q_PROPERTY(QString submitButtonText READ submitButtonText WRITE setSubmitButtonText NOTIFY submitButtonTextChanged)
    /*!
     * \property KNotificationReplyAction::submitButtonIconName
     * \since 5.88
     */
    Q_PROPERTY(QString submitButtonIconName READ submitButtonIconName WRITE setSubmitButtonIconName NOTIFY submitButtonIconNameChanged)
    /*!
     * \property KNotificationReplyAction::fallbackBehavior
     * \since 5.88
     */
    Q_PROPERTY(FallbackBehavior fallbackBehavior READ fallbackBehavior WRITE setFallbackBehavior NOTIFY fallbackBehaviorChanged)

public:
    /*!
     * Creates a inline reply action with given label
     *
     * \a label The label for the action
     */
    explicit KNotificationReplyAction(const QString &label);

    ~KNotificationReplyAction() override;

    /*!
     * The label for the action button
     */
    QString label() const;
    /*!
     * Set the label for the action button
     */
    void setLabel(const QString &label);

    /*!
     * The placeholder text for the inline reply text field
     */
    QString placeholderText() const;
    /*!
     * Set the placeholder text for the inline reply text field, for example "Reply to Konqi..."
     */
    void setPlaceholderText(const QString &placeholderText);

    /*!
     * The label for the button to send the typed reply
     */
    QString submitButtonText() const;
    /*!
     * Set the label for the button to send the typed reply
     */
    void setSubmitButtonText(const QString &submitButtonText);

    /*!
     * The icon name for the button to send the typed reply
     */
    QString submitButtonIconName() const;
    /*!
     * Set the icon name for the button to send the typed reply
     */
    void setSubmitButtonIconName(const QString &submitButtonIconName);

    /*!
     * Behavior when the notification server does not support inline replies
     *
     * \value HideAction Don't add the reply action (default)
     * \value UseRegularAction Add the reply action as regular button
     * Use this if you want to provide your own reply functionality
     * Note: The activated() signal is emitted instead of replied()
     */
    enum class FallbackBehavior {
        HideAction,
        UseRegularAction,
    };
    Q_ENUM(FallbackBehavior)

    /*!
     * Gets the fallback behavior when the notification server does not support inline replies
     */
    FallbackBehavior fallbackBehavior() const;
    /*!
     * Set the fallback behavior for when the notification server does not support inline replies
     */
    void setFallbackBehavior(FallbackBehavior fallbackBehavior);

Q_SIGNALS:
    /*!
     * Emitted when the user has submitted a reply
     *
     * \note This is never emitted when the notification server does not support inline replies
     *
     * \a text The text the user entered
     */
    void replied(const QString &text);
    /*!
     * Emitted when the user clicks the reply fallback button
     *
     * \note This is emitted when the notification server does not support inline replies
     * and fallbackBehavior is set to UseRegularAction.
     */
    void activated();

    /*!
     * Emitted when label changed.
     * \since 5.88
     */
    void labelChanged();
    /*!
     * Emitted when placeholderText changed.
     * \since 5.88
     */
    void placeholderTextChanged();
    /*!
     * Emitted when submitButtonText changed.
     * \since 5.88
     */
    void submitButtonTextChanged();
    /*!
     * Emitted when submitButtonIconName changed.
     * \since 5.88
     */
    void submitButtonIconNameChanged();
    /*!
     * Emitted when fallbackBehavior changed.
     * \since 5.88
     */
    void fallbackBehaviorChanged();

private:
    std::unique_ptr<KNotificationReplyActionPrivate> const d;
};

#endif // KNOTIFICATIONREPLYACTION_H
