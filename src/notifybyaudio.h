/* This file is part of the KDE libraries
   Copyright 2014 by Martin Klapetek <mklapetek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#ifndef NOTIFYBYLOGFILE_H
#define NOTIFYBYLOGFILE_H

#include "knotifyplugin.h"

#include <phonon/MediaObject>

namespace Phonon {
// class MediaObject;
class MediaSource;
class AudioOutput;
}

class KNotification;

class NotifyByAudio : public KNotifyPlugin
{
    Q_OBJECT

public:
    NotifyByAudio(QObject *parent = 0);
    virtual ~NotifyByAudio();

    virtual QString optionName() { return QStringLiteral("Sound"); }
    virtual void notify(KNotification *notification, KNotifyConfig *config);
    virtual void close(KNotification *notification);

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
