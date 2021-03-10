/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014-2015 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "notifybyaudio_canberra.h"
#include "debug_p.h"

#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QString>

#include "knotification.h"
#include "knotifyconfig.h"

#include <canberra.h>

NotifyByAudio::NotifyByAudio(QObject *parent)
    : KNotificationPlugin(parent)
{
    qRegisterMetaType<uint32_t>("uint32_t");

    int ret = ca_context_create(&m_context);
    if (ret != CA_SUCCESS) {
        qCWarning(LOG_KNOTIFICATIONS) << "Failed to initialize canberra context for audio notification:" << ca_strerror(ret);
        m_context = nullptr;
        return;
    }

    QString desktopFileName = QGuiApplication::desktopFileName();
    // handle apps which set the desktopFileName property with filename suffix,
    // due to unclear API dox (https://bugreports.qt.io/browse/QTBUG-75521)
    if (desktopFileName.endsWith(QLatin1String(".desktop"))) {
        desktopFileName.chop(8);
    }
    ret = ca_context_change_props(m_context,
                                  CA_PROP_APPLICATION_NAME,
                                  qUtf8Printable(qApp->applicationDisplayName()),
                                  CA_PROP_APPLICATION_ID,
                                  qUtf8Printable(desktopFileName),
                                  CA_PROP_APPLICATION_ICON_NAME,
                                  qUtf8Printable(qApp->windowIcon().name()),
                                  nullptr);
    if (ret != CA_SUCCESS) {
        qCWarning(LOG_KNOTIFICATIONS) << "Failed to set application properties on canberra context for audio notification:" << ca_strerror(ret);
    }
}

NotifyByAudio::~NotifyByAudio()
{
    if (m_context) {
        ca_context_destroy(m_context);
    }
    m_context = nullptr;
}

void NotifyByAudio::notify(KNotification *notification, KNotifyConfig *config)
{
    const QString soundFilename = config->readEntry(QStringLiteral("Sound"));
    if (soundFilename.isEmpty()) {
        qCWarning(LOG_KNOTIFICATIONS) << "Audio notification requested, but no sound file provided in notifyrc file, aborting audio notification";

        finish(notification);
        return;
    }

    QUrl soundURL;
    const auto dataLocations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &dataLocation : dataLocations) {
        soundURL = QUrl::fromUserInput(soundFilename, dataLocation + QStringLiteral("/sounds"), QUrl::AssumeLocalFile);
        if (soundURL.isLocalFile() && QFileInfo::exists(soundURL.toLocalFile())) {
            break;
        } else if (!soundURL.isLocalFile() && soundURL.isValid()) {
            break;
        }
        soundURL.clear();
    }
    if (soundURL.isEmpty()) {
        qCWarning(LOG_KNOTIFICATIONS) << "Audio notification requested, but sound file from notifyrc file was not found, aborting audio notification";
        finish(notification);
        return;
    }

    // Looping happens in the finishCallback
    if (!playSound(m_currentId, soundURL)) {
        finish(notification);
        return;
    }

    if (notification->flags() & KNotification::LoopSound) {
        m_loopSoundUrls.insert(m_currentId, soundURL);
    }

    Q_ASSERT(!m_notifications.value(m_currentId));
    m_notifications.insert(m_currentId, notification);

    ++m_currentId;
}

bool NotifyByAudio::playSound(quint32 id, const QUrl &url)
{
    if (!m_context) {
        qCWarning(LOG_KNOTIFICATIONS) << "Cannot play notification sound without canberra context";
        return false;
    }

    ca_proplist *props = nullptr;
    ca_proplist_create(&props);

    // We'll also want this cached for a time. volatile makes sure the cache is
    // dropped after some time or when the cache is under pressure.
    ca_proplist_sets(props, CA_PROP_MEDIA_FILENAME, QFile::encodeName(url.toLocalFile()).constData());
    ca_proplist_sets(props, CA_PROP_CANBERRA_CACHE_CONTROL, "volatile");

    int ret = ca_context_play_full(m_context, id, props, &ca_finish_callback, this);

    ca_proplist_destroy(props);

    if (ret != CA_SUCCESS) {
        qCWarning(LOG_KNOTIFICATIONS) << "Failed to play sound with canberra:" << ca_strerror(ret);
        return false;
    }

    return true;
}

void NotifyByAudio::ca_finish_callback(ca_context *c, uint32_t id, int error_code, void *userdata)
{
    Q_UNUSED(c);
    QMetaObject::invokeMethod(static_cast<NotifyByAudio *>(userdata), "finishCallback", Q_ARG(uint32_t, id), Q_ARG(int, error_code));
}

void NotifyByAudio::finishCallback(uint32_t id, int error_code)
{
    KNotification *notification = m_notifications.value(id, nullptr);
    if (!notification) {
        // We may have gotten a late finish callback.
        return;
    }

    if (error_code == CA_SUCCESS) {
        // Loop the sound now if we have one
        const QUrl soundUrl = m_loopSoundUrls.value(id);
        if (soundUrl.isValid()) {
            if (!playSound(id, soundUrl)) {
                finishNotification(notification, id);
            }
            return;
        }
    } else if (error_code != CA_ERROR_CANCELED) {
        qCWarning(LOG_KNOTIFICATIONS) << "Playing audio notification failed:" << ca_strerror(error_code);
    }

    finishNotification(notification, id);
}

void NotifyByAudio::close(KNotification *notification)
{
    if (!m_notifications.values().contains(notification)) {
        return;
    }

    const auto id = m_notifications.key(notification);
    if (m_context) {
        int ret = ca_context_cancel(m_context, id);
        if (ret != CA_SUCCESS) {
            qCWarning(LOG_KNOTIFICATIONS) << "Failed to cancel canberra context for audio notification:" << ca_strerror(ret);
            return;
        }
    }

    // Consider the notification finished. ca_context_cancel schedules a cancel
    // but we need to stop using the noficiation immediately or we could access
    // a notification past its lifetime (close() may, or indeed must,
    // schedule deletion of the notification).
    // https://bugs.kde.org/show_bug.cgi?id=398695
    finishNotification(notification, id);
}

void NotifyByAudio::finishNotification(KNotification *notification, quint32 id)
{
    m_notifications.remove(id);
    m_loopSoundUrls.remove(id);
    finish(notification);
}
