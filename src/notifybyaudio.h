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

#include <phonon/MediaObject>

namespace Phonon {
// class MediaObject;
class MediaSource;
class AudioOutput;
}

class KNotification;

class NotifyByAudio : public KNotificationPlugin
{
    Q_OBJECT

public:
    NotifyByAudio(QObject *parent = 0);
    virtual ~NotifyByAudio();

    QString optionName() Q_DECL_OVERRIDE { return QStringLiteral("Sound"); }
    void notify(KNotification *notification, KNotifyConfig *config) Q_DECL_OVERRIDE;
    void close(KNotification *notification) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void onAudioFinished();
    void onAudioSourceChanged(const Phonon::MediaSource &source);
    void stateChanged(Phonon::State newState, Phonon::State oldState);


private:
    QList<Phonon::MediaObject*> m_reusablePhonons;
    QHash<Phonon::MediaObject*, KNotification*> m_notifications;
    Phonon::AudioOutput *m_audioOutput;
};

#endif
