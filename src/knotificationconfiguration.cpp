/*
    SPDX-FileCopyrightText: 2020-2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knotificationconfiguration.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

using namespace Qt::Literals;

[[nodiscard]] static bool hasNotifyRcFile(QStringView appName)
{
    if (!QStandardPaths::locate(QStandardPaths::GenericDataLocation, "knotifications6/"_L1 + appName + ".notifyrc"_L1).isEmpty()) {
        return true;
    }
#ifdef Q_OS_ANDROID
    // on platforms where the system doesn't read notifyrc files bundling them as Qt resources is good enough'
    if (QFileInfo::exists(":/knotifications6/"_L1 + appName + ".notifyrc"_L1)) {
        return true;
    }
#endif
    return false;
}

#ifndef Q_OS_ANDROID
struct {
    bool initialized : 1 = false;
    bool hasSystemSettingsUri : 1 = false;
    bool hasKcm : 1 = false;
} static s_capabilities;

static void detectCapabilities()
{
    if (s_capabilities.initialized) {
        return;
    }
    s_capabilities.initialized = true;

    if (!hasNotifyRcFile(QCoreApplication::applicationName())) {
        return;
    }

    if (QFileInfo::exists(u"/.flatpak-info"_s)) {
        // technically only for Plasma >= 6.7, but we have no way to detect that
        s_capabilities.hasSystemSettingsUri = qgetenv("XDG_CURRENT_DESKTOP") == "KDE";
        return;
    }

    s_capabilities.hasSystemSettingsUri = !QStandardPaths::findExecutable(u"systemsettings"_s).isEmpty();

    if (!s_capabilities.hasSystemSettingsUri) {
        QProcess proc;
        proc.start(u"kcmshell6"_s, {u"--list"_s});
        proc.waitForFinished();
        s_capabilities.hasKcm = proc.readAllStandardOutput().contains("\nkcm_notifications "_L1);
    }
}
#endif

bool KNotificationConfiguration::isAvailable()
{
#ifdef Q_OS_ANDROID
    return hasNotifyRcFile(QCoreApplication::applicationName());
#else
    detectCapabilities();
    return s_capabilities.hasSystemSettingsUri || s_capabilities.hasKcm;
#endif
}

void KNotificationConfiguration::show()
{
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>("org/kde/knotifications/KNotificationConfiguration", "show", QNativeInterface::QAndroidApplication::context());
#else
    detectCapabilities();
    if (s_capabilities.hasSystemSettingsUri) {
        QUrl url;
        url.setScheme(u"systemsettings"_s);
        url.setHost(u"kcm_notifications"_s);
        url.setPath("/--notifyrc "_L1 + QCoreApplication::applicationName());
        QDesktopServices::openUrl(url);
        return;
    }

    if (s_capabilities.hasKcm) {
        QProcess proc;
        proc.setProgram(u"kcmshell6"_s);
        proc.setArguments({u"--args"_s, "--notifyrc %1"_L1.arg(QCoreApplication::applicationName()), u"notifications"_s});
        proc.startDetached();
    }
#endif
}

#include "moc_knotificationconfiguration.cpp"
