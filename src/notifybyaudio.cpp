/* This file is part of the KDE libraries
   Copyright (C) 2014-2015 by Martin Klapetek <mklapetek@kde.org>

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

#include "notifybyaudio.h"

#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QString>

#include "knotifyconfig.h"
#include "knotification.h"

#include <phonon/mediaobject.h>
#include <phonon/mediasource.h>
#include <phonon/audiooutput.h>

NotifyByAudio::NotifyByAudio(QObject *parent)
    : KNotificationPlugin(parent),
      m_audioOutput(nullptr)
{
}

NotifyByAudio::~NotifyByAudio()
{
    qDeleteAll(m_reusablePhonons);
    delete m_audioOutput;
}

void NotifyByAudio::notify(KNotification *notification, KNotifyConfig *config)
{
    if (!m_audioOutput) {
        m_audioOutput = new Phonon::AudioOutput(Phonon::NotificationCategory, this);
    }
    QString soundFilename = config->readEntry("Sound");
    if (soundFilename.isEmpty()) {
        qWarning() << "Audio notification requested, but no sound file provided in notifyrc file, aborting audio notification";

        finish(notification);
        return;
    }

    QUrl soundURL = QUrl(soundFilename); // this CTOR accepts both absolute paths (/usr/share/sounds/blabla.ogg and blabla.ogg) w/o screwing the scheme
    if (soundURL.isRelative() && !soundURL.toString().startsWith('/')) { // QUrl considers url.scheme.isEmpty() == url.isRelative()
        soundURL = QUrl::fromLocalFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("sounds/") + soundFilename));

        if (soundURL.isEmpty()) {
            qWarning() << "Audio notification requested, but sound file from notifyrc file was not found, aborting audio notification";

            finish(notification);
            return;
        }
    }

    Phonon::MediaObject *m;

    if (m_reusablePhonons.isEmpty()) {
        m = new Phonon::MediaObject(this);
        connect(m, SIGNAL(finished()), SLOT(onAudioFinished()));
        connect(m, SIGNAL(stateChanged(Phonon::State,Phonon::State)), SLOT(stateChanged(Phonon::State,Phonon::State)));
        Phonon::createPath(m, m_audioOutput);
    } else {
        m = m_reusablePhonons.takeFirst();
    }

    m->setCurrentSource(soundURL);
    m->play();

    if (notification->flags() & KNotification::LoopSound) {
        // Enqueing essentially prevents the subsystem pipeline from partial teardown
        // which is the most desired thing in terms of load and delay between loop cycles.
        // All of this is timing dependent, which is why we want at least one source queued;
        // in reality the shorter the source the more sources we want to be queued to prevent
        // the MO from running out of sources.
        // Point being that all phonon signals are forcefully queued (becuase qthread has problems detecting !pthread threads),
        // so when you get for example the aboutToFinish signal the MO might already have stopped playing.
        //
        // And so we queue it three times at least; doesn't cost anything and keeps us safe.

        m->enqueue(soundURL);
        m->enqueue(soundURL);
        m->enqueue(soundURL);

        connect(m, SIGNAL(currentSourceChanged(Phonon::MediaSource)), SLOT(onAudioSourceChanged(Phonon::MediaSource)));
    }

    m_notifications.insert(m, notification);
}

void NotifyByAudio::stateChanged(Phonon::State newState, Phonon::State oldState)
{
    qDebug() << "Changing audio state from" << oldState << "to" << newState;
}

void NotifyByAudio::close(KNotification *notification)
{
    Phonon::MediaObject *m = m_notifications.key(notification);

    if (!m) {
        return;
    }

    // this should call onAudioFinished() which also does finish() on the notification
    m->stop();
    m_reusablePhonons.append(m);
}

void NotifyByAudio::onAudioFinished()
{
    Phonon::MediaObject *m = qobject_cast<Phonon::MediaObject*>(sender());

    if (!m) {
        return;
    }

    KNotification *notification = m_notifications.value(m);

    finish(notification);

    disconnect(m, SIGNAL(currentSourceChanged(Phonon::MediaSource)), this, SLOT(onAudioSourceChanged(Phonon::MediaSource)));

    m_notifications.remove(m);
    m_reusablePhonons.append(m);
}

void NotifyByAudio::onAudioSourceChanged(const Phonon::MediaSource &source)
{
    Phonon::MediaObject *m = qobject_cast<Phonon::MediaObject*>(sender());

    if (!m) {
        return;
    }

    m->enqueue(source);
}

