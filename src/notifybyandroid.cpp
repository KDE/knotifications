/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "notifybyandroid.h"
#include "knotification.h"
#include "knotifyconfig.h"
#include "debug_p.h"

#include <QtAndroid>
#include <QAndroidJniEnvironment>

#include <QBuffer>
#include <QDebug>
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

static const JNINativeMethod methods[] = {
    {"notificationFinished", "(I)V", (void*)notificationFinished},
    {"notificationActionInvoked", "(II)V", (void*)notificationActionInvoked}
};

KNOTIFICATIONS_EXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void*)
{
    static bool initialized = false;
    if (initialized) {
        return JNI_VERSION_1_4;
    }
    initialized = true;

    JNIEnv *env = nullptr;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
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

NotifyByAndroid::NotifyByAndroid(QObject* parent) :
    KNotificationPlugin(parent)
{
    s_instance = this;
    m_backend = QAndroidJniObject("org/kde/knotifications/NotifyByAndroid", "(Landroid/content/Context;)V", QtAndroid::androidContext().object<jobject>());
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
    QMetaObject::invokeMethod(this, [this, notification](){ notifyDeferred(notification); }, Qt::QueuedConnection);
}

void NotifyByAndroid::notifyDeferred(KNotification* notification)
{
    KNotifyConfig config(notification->appName(), notification->contexts(), notification->eventId());
    QAndroidJniEnvironment env;

    QAndroidJniObject n("org/kde/knotifications/KNotification", "()V");
    n.setField("id", notification->id());
    n.setField("text", QAndroidJniObject::fromString(notification->text()).object<jstring>());
    n.setField("title", QAndroidJniObject::fromString(notification->title()).object<jstring>());

    n.setField("channelId", QAndroidJniObject::fromString(notification->eventId()).object<jstring>());
    n.setField("channelName", QAndroidJniObject::fromString(config.readEntry(QLatin1String("Name"))).object<jstring>());
    n.setField("channelDescription", QAndroidJniObject::fromString(config.readEntry(QLatin1String("Comment"))).object<jstring>());

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
    env->SetByteArrayRegion(jIconData, 0, iconData.length(), reinterpret_cast<const jbyte*>(iconData.constData()));
    n.callMethod<void>("setIconFromData", "([BI)V", jIconData, iconData.length());

    // actions
    const auto actions = notification->actions();
    for (const auto &action : actions) {
        n.callMethod<void>("addAction", "(Ljava/lang/String;)V", QAndroidJniObject::fromString(action).object<jstring>());
    }

    m_notifications.insert(notification->id(), notification);

    m_backend.callMethod<void>("notify", "(Lorg/kde/knotifications/KNotification;)V", n.object<jobject>());
}

void NotifyByAndroid::close(KNotification* notification)
{
    m_backend.callMethod<void>("close", "(I)V", notification->id());
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
    emit actionInvoked(id, action);
}
