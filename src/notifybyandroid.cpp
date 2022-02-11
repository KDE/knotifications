/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "notifybyandroid.h"
#include "debug_p.h"
#include "knotification.h"
#include "knotificationreplyaction.h"
#include "knotifyconfig.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#else
#include <QCoreApplication>
#include <QJniEnvironment>
// TODO KF6 remove this porting aid
using QAndroidJniEnvironment = QJniEnvironment;
#endif

#include <QBuffer>
#include <QIcon>

static NotifyByAndroid *s_instance = nullptr;

static void notificationFinished(JNIEnv *env, jobject that, jint notificationId)
{
    Q_UNUSED(env);
    Q_UNUSED(that);
    if (s_instance) {
        s_instance->notificationFinished(notificationId);
    }
}

static void notificationActionInvoked(JNIEnv *env, jobject that, jint id, jint action)
{
    Q_UNUSED(env);
    Q_UNUSED(that);
    if (s_instance) {
        s_instance->notificationActionInvoked(id, action);
    }
}

static void notificationInlineReply(JNIEnv *env, jobject that, jint id, jstring text)
{
    Q_UNUSED(that);
    if (s_instance) {
        const char *str = env->GetStringUTFChars(text, nullptr);
        s_instance->notificationInlineReply(id, QString::fromUtf8(str));
        env->ReleaseStringUTFChars(text, str);
    }
}

static const JNINativeMethod methods[] = {
    {"notificationFinished", "(I)V", (void *)notificationFinished},
    {"notificationActionInvoked", "(II)V", (void *)notificationActionInvoked},
    {"notificationInlineReply", "(ILjava/lang/String;)V", (void *)notificationInlineReply},
};

KNOTIFICATIONS_EXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *)
{
    static bool initialized = false;
    if (initialized) {
        return JNI_VERSION_1_4;
    }
    initialized = true;

    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **)&env, JNI_VERSION_1_4) != JNI_OK) {
        qCWarning(LOG_KNOTIFICATIONS) << "Failed to get JNI environment.";
        return -1;
    }
    jclass cls = env->FindClass("org/kde/knotifications/NotifyByAndroid");
    if (env->RegisterNatives(cls, methods, sizeof(methods) / sizeof(JNINativeMethod)) < 0) {
        qCWarning(LOG_KNOTIFICATIONS) << "Failed to register native functions.";
        return -1;
    }

    return JNI_VERSION_1_4;
}

NotifyByAndroid::NotifyByAndroid(QObject *parent)
    : KNotificationPlugin(parent)
{
    s_instance = this;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QAndroidJniObject context = QtAndroid::androidContext();
#else
    QJniObject context = QNativeInterface::QAndroidApplication::context();
#endif
    m_backend = QAndroidJniObject("org/kde/knotifications/NotifyByAndroid", "(Landroid/content/Context;)V", context.object<jobject>());
}

NotifyByAndroid::~NotifyByAndroid()
{
    s_instance = nullptr;
}

QString NotifyByAndroid::optionName()
{
    return QStringLiteral("Popup");
}

void NotifyByAndroid::notify(KNotification *notification, KNotifyConfig *config)
{
    Q_UNUSED(config);
    // HACK work around that notification->id() is only populated after returning from here
    // note that config will be invalid at that point, so we can't pass that along
    QMetaObject::invokeMethod(
        this,
        [this, notification]() {
            notifyDeferred(notification);
        },
        Qt::QueuedConnection);
}

QAndroidJniObject NotifyByAndroid::createAndroidNotification(KNotification *notification, KNotifyConfig *config) const
{
    QAndroidJniEnvironment env;
    QAndroidJniObject n("org/kde/knotifications/KNotification", "()V");
    n.setField("id", notification->id());
    n.setField("text", QAndroidJniObject::fromString(stripRichText(notification->text())).object<jstring>());
    n.setField("richText", QAndroidJniObject::fromString(notification->text()).object<jstring>());
    n.setField("title", QAndroidJniObject::fromString(notification->title()).object<jstring>());
    n.setField("urgency", (jint)(notification->urgency() == KNotification::DefaultUrgency ? KNotification::HighUrgency : notification->urgency()));
    n.setField("visibility",
               QAndroidJniObject::fromString(notification->hints().value(QLatin1String("x-kde-visibility")).toString().toLower()).object<jstring>());

    n.setField("channelId", QAndroidJniObject::fromString(notification->eventId()).object<jstring>());
    n.setField("channelName", QAndroidJniObject::fromString(config->readEntry(QLatin1String("Name"))).object<jstring>());
    n.setField("channelDescription", QAndroidJniObject::fromString(config->readEntry(QLatin1String("Comment"))).object<jstring>());

    if ((notification->flags() & KNotification::SkipGrouping) == 0) {
        n.setField("group", QAndroidJniObject::fromString(notification->eventId()).object<jstring>());
    }

    // icon
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
    auto jIconData = env->NewByteArray(iconData.length());
    env->SetByteArrayRegion(jIconData, 0, iconData.length(), reinterpret_cast<const jbyte *>(iconData.constData()));
    n.callMethod<void>("setIconFromData", "([BI)V", jIconData, iconData.length());
    env->DeleteLocalRef(jIconData);

    // actions
    const auto actions = notification->actions();
    for (const auto &action : actions) {
        n.callMethod<void>("addAction", "(Ljava/lang/String;)V", QAndroidJniObject::fromString(action).object<jstring>());
    }

    if (notification->replyAction()) {
        n.setField("inlineReplyLabel", QAndroidJniObject::fromString(notification->replyAction()->label()).object<jstring>());
        n.setField("inlineReplyPlaceholder", QAndroidJniObject::fromString(notification->replyAction()->placeholderText()).object<jstring>());
    }

    return n;
}

void NotifyByAndroid::notifyDeferred(KNotification *notification)
{
    KNotifyConfig config(notification->appName(), notification->contexts(), notification->eventId());
    const auto n = createAndroidNotification(notification, &config);
    m_notifications.insert(notification->id(), notification);

    m_backend.callMethod<void>("notify", "(Lorg/kde/knotifications/KNotification;)V", n.object<jobject>());
}

void NotifyByAndroid::update(KNotification *notification, KNotifyConfig *config)
{
    const auto n = createAndroidNotification(notification, config);
    m_backend.callMethod<void>("notify", "(Lorg/kde/knotifications/KNotification;)V", n.object<jobject>());
}

void NotifyByAndroid::close(KNotification *notification)
{
    m_backend.callMethod<void>("close", "(ILjava/lang/String;)V", notification->id(), QAndroidJniObject::fromString(notification->eventId()).object<jstring>());
    KNotificationPlugin::close(notification);
}

void NotifyByAndroid::notificationFinished(int id)
{
    qCDebug(LOG_KNOTIFICATIONS) << id;
    const auto it = m_notifications.find(id);
    if (it == m_notifications.end()) {
        return;
    }
    m_notifications.erase(it);
    if (it.value()) {
        finish(it.value());
    }
}

void NotifyByAndroid::notificationActionInvoked(int id, int action)
{
    qCDebug(LOG_KNOTIFICATIONS) << id << action;
    Q_EMIT actionInvoked(id, action);
}

void NotifyByAndroid::notificationInlineReply(int id, const QString &text)
{
    qCDebug(LOG_KNOTIFICATIONS) << id << text;
    Q_EMIT replied(id, text);
}
