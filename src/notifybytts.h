/*
    SPDX-FileCopyrightText: 2007 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2009 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NOTIFYBYTTS_H
#define NOTIFYBYTTS_H

#include "knotificationplugin.h"

class QTextToSpeech;

class NotifyByTTS : public KNotificationPlugin
{
    Q_OBJECT
public:
    explicit NotifyByTTS(QObject *parent = nullptr);
    ~NotifyByTTS() override;

    QString optionName() override
    {
        return QStringLiteral("TTS");
    }
    void notify(KNotification *notification, KNotifyConfig *config) override;

private:
    QTextToSpeech *const m_speech;
};

#endif
