/*
   Copyright (C) 2007 by Olivier Goffart <ogoffart at kde.org>
   Copyright (C) 2009 by Laurent Montel <montel@kde.org>
   Copyright (C) 2015 by Jeremy Whiting <jpwhiting@kde.org>

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

#include "notifybytts.h"
#include <QTextToSpeech>
#include <KMacroExpander>
#include "knotifyconfig.h"
#include "knotification.h"
#include "debug_p.h"

#include <QtDebug>

NotifyByTTS::NotifyByTTS(QObject *parent)
    : KNotificationPlugin(parent),
      m_speech(0)
{
    m_speech = new QTextToSpeech(this);
}


NotifyByTTS::~NotifyByTTS()
{
    delete m_speech;
    m_speech = 0;
}

void NotifyByTTS::notify(KNotification *notification, KNotifyConfig *config )
{
    if (m_speech->state() != QTextToSpeech::BackendError) {
        QString say = config->readEntry( QStringLiteral("TTS") );

        if (!say.isEmpty()) {
            // Create a hash of characters to strings to expand text into the notification text.
            QHash<QChar,QString> subst;
            subst.insert('e', notification->eventId());
            subst.insert('a', notification->appName());
            subst.insert('s', notification->text());
            //subst.insert('w', QString::number((quintptr)config->winId));
            //subst.insert('i', QString::number(id));
            subst.insert('m', notification->text());
            say = KMacroExpander::expandMacrosShellQuote( say, subst );
        }

        if (say.isEmpty())
            say = notification->text(); // fallback on the plain text

        m_speech->say(say);

        finished(notification);
    } else {
        qCDebug(LOG_KNOTIFICATIONS) << "Speech backend has an error, not speaking";
    }
}
