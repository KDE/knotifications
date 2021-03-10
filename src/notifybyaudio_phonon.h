/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NOTIFYBYAUDIO_H
#define NOTIFYBYAUDIO_H

#include "knotificationplugin.h"

#include <phonon/MediaObject>

namespace Phonon
{
// class MediaObject;
class MediaSource;
class AudioOutput;
}

class KNotification;

class NotifyByAudio : public KNotificationPlugin
{
    Q_OBJECT

public:
    explicit NotifyByAudio(QObject *parent = nullptr);
    ~NotifyByAudio() override;

    QString optionName() override
    {
        return QStringLiteral("Sound");
    }
    void notify(KNotification *notification, KNotifyConfig *config) override;
    void close(KNotification *notification) override;

private Q_SLOTS:
    void onAudioFinished();
    void onAudioSourceChanged(const Phonon::MediaSource &source);
    void stateChanged(Phonon::State newState, Phonon::State oldState);

private:
    void finishNotification(KNotification *notification, Phonon::MediaObject *m);

    QList<Phonon::MediaObject *> m_reusablePhonons;
    QHash<Phonon::MediaObject *, KNotification *> m_notifications;
    Phonon::AudioOutput *m_audioOutput;
};

#endif
