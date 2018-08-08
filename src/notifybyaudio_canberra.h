/* This file is part of the KDE libraries
   Copyright 2014 by Martin Klapetek <mklapetek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/


#ifndef NOTIFYBYAUDIO_H
#define NOTIFYBYAUDIO_H

#include "knotificationplugin.h"

#include <QHash>
#include <QUrl>

class KNotification;

struct ca_context;

class NotifyByAudio : public KNotificationPlugin
{
    Q_OBJECT

public:
    explicit NotifyByAudio(QObject *parent = nullptr);
    ~NotifyByAudio() override;

    QString optionName() override { return QStringLiteral("Sound"); }
    void notify(KNotification *notification, KNotifyConfig *config) override;
    void close(KNotification *notification) override;

private Q_SLOTS:
    void finishCallback(uint32_t id,
                        int error_code);

private:
    static void ca_finish_callback(ca_context *c,
                                   uint32_t id,
                                   int error_code,
                                   void *userdata);

    void finishNotification(KNotification *notification, quint32 id);

    bool playSound(quint32 id, const QUrl &url);

    ca_context *m_context = nullptr;
    quint32 m_currentId = 0;
    QHash<quint32, KNotification*> m_notifications;
    // in case we loop we store the URL for the notification to be able to replay it
    QHash<quint32, QUrl> m_loopSoundUrls;
};

#endif
