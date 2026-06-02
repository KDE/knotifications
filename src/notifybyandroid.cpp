/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "notifybyandroid.h"
#include "debug_p.h"
#include "knotification.h"
#include "knotificationreplyaction.h"
#include "knotifyconfig.h"

#include <QCoreApplication>
#include <QJniEnvironment>

#include <QBuffer>
#include <QIcon>

using namespace Qt::Literals;
using namespace QtJniTypes;

static NotifyByAndroid *s_instance = nullptr;

static void notificationFinished(JNIEnv *env, jobject that, jint notificationId)
{
    Q_UNUSED(env);
    Q_UNUSED(that);
    if (s_instance) {
        s_instance->notificationFinished(notificationId);
    }
}

Q_DECLARE_JNI_NATIVE_METHOD(notificationFinished);

static void notificationActionInvoked(JNIEnv *env, jobject that, jint id, const QString &action)
{
    Q_UNUSED(env);
    Q_UNUSED(that);
    if (s_instance) {
        s_instance->notificationActionInvoked(id, action);
    }
}

Q_DECLARE_JNI_NATIVE_METHOD(notificationActionInvoked)

static void notificationInlineReply(JNIEnv *env, jobject that, jint id, const QString &text)
{
    Q_UNUSED(env);
    Q_UNUSED(that);
    if (s_instance) {
        s_instance->notificationInlineReply(id, text);
    }
}

Q_DECLARE_JNI_NATIVE_METHOD(notificationInlineReply)

NotifyByAndroid::NotifyByAndroid(QObject *parent)
    : KNotificationPlugin(parent)
{
    static bool nativeCallBacksInitialized = []() {
        return NotifyByAndroidBackend::registerNativeMethods({
            Q_JNI_NATIVE_METHOD(notificationFinished),
            Q_JNI_NATIVE_METHOD(notificationActionInvoked),
            Q_JNI_NATIVE_METHOD(notificationInlineReply)
        });
    }();

    s_instance = this;
    m_backend = NotifyByAndroidBackend(QNativeInterface::QAndroidApplication::context());
}

NotifyByAndroid::~NotifyByAndroid()
{
    s_instance = nullptr;
}

QString NotifyByAndroid::optionName()
{
    return QStringLiteral("Popup");
}

KNotificationData NotifyByAndroid::createAndroidNotification(KNotification *notification, const KNotifyConfig &notifyConfig) const
{
    QJniEnvironment env;
    KNotificationData n;
    n.setField("id", notification->id());
    n.setField("text", stripRichText(notification->text()));
    n.setField("richText", notification->text());
    n.setField("title", notification->title());
    n.setField("urgency", (jint)(notification->urgency() == KNotification::DefaultUrgency ? KNotification::HighUrgency : notification->urgency()));
    n.setField("visibility", notification->hints().value(QLatin1String("x-kde-visibility")).toString().toLower());

    n.setField("channelId", notification->eventId());
    n.setField("channelName", notifyConfig.readEntry(QLatin1String("Name")));
    n.setField("channelDescription", notifyConfig.readEntry(QLatin1String("Comment")));

    if ((notification->flags() & KNotification::SkipGrouping) == 0) {
        n.setField("group", notification->eventId());
    }

    // icon
    {
        QPixmap pixmap;
        if (!notification->iconName().isEmpty()) {
            const auto icon = QIcon::fromTheme(notification->iconName());
            pixmap = icon.pixmap(32, 32);
        } else {
            pixmap = notification->pixmap();
        }
        QByteArray iconData;
        QBuffer buffer(&iconData);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
        n.callMethod<void>("setIconFromData", iconData);
    }

    // (symbolic) application icon
    {
        QPixmap pixmap;
        const auto appIconHint = notification->hints().value("x-kde-symbolic-app-icon"_L1);
        if (appIconHint.typeId() == QMetaType::QString) {
            const auto icon = QIcon::fromTheme(appIconHint.toString());
            pixmap = icon.pixmap(32, 32);
        } else if (appIconHint.typeId() == QMetaType::QIcon) {
            pixmap = appIconHint.value<QIcon>().pixmap(32, 32);
        } else if (appIconHint.typeId() == QMetaType::QPixmap) {
            pixmap = appIconHint.value<QPixmap>();
        }

        if (!pixmap.isNull()) {
            QByteArray iconData;
            QBuffer buffer(&iconData);
            buffer.open(QIODevice::WriteOnly);
            pixmap.save(&buffer, "PNG");
            n.callMethod<void>("setAppIconFromData", iconData);
        }
    }

    // actions
    const auto actions = notification->actions();
    for (const KNotificationAction *action : actions) {
        n.callMethod<void>("addAction", action->id(), action->label());
    }

    if (notification->replyAction()) {
        n.setField("inlineReplyLabel", notification->replyAction()->label());
        n.setField("inlineReplyPlaceholder", notification->replyAction()->placeholderText());
    }

    return n;
}

void NotifyByAndroid::notify(KNotification *notification, const KNotifyConfig &notifyConfig)
{
    const auto n = createAndroidNotification(notification, notifyConfig);
    m_notifications.insert(notification->id(), notification);

    m_backend.callMethod<void>("notify", n);
}

void NotifyByAndroid::update(KNotification *notification, const KNotifyConfig &notifyConfig)
{
    const auto n = createAndroidNotification(notification, notifyConfig);
    m_backend.callMethod<void>("notify", n);
}

void NotifyByAndroid::close(KNotification *notification)
{
    m_backend.callMethod<void>("close", notification->id(), notification->eventId());
    KNotificationPlugin::close(notification);
}

void NotifyByAndroid::notificationFinished(int id)
{
    qCDebug(LOG_KNOTIFICATIONS) << id;
    const auto it = m_notifications.find(id);
    if (it == m_notifications.end()) {
        return;
    }
    const auto n = it.value();
    m_notifications.erase(it);
    if (n) {
        finish(n);
    }
}

void NotifyByAndroid::notificationActionInvoked(int id, const QString &action)
{
    qCDebug(LOG_KNOTIFICATIONS) << id << action;
    Q_EMIT actionInvoked(id, action);
}

void NotifyByAndroid::notificationInlineReply(int id, const QString &text)
{
    qCDebug(LOG_KNOTIFICATIONS) << id << text;
    Q_EMIT replied(id, text);

    // confirm we got the reply, and thus stop the spinner animation
    const auto it = m_notifications.constFind(id);
    if (it != m_notifications.end() && it.value()) {
        KNotifyConfig config(it.value()->appName(), it.value()->eventId());
        notify(it.value(), config);
    }
}

#include "moc_notifybyandroid.cpp"
