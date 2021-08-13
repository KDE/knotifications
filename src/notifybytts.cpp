/*
    SPDX-FileCopyrightText: 2007 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2009 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2015 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "notifybytts.h"
#include "debug_p.h"
#include "knotification.h"
#include "knotifyconfig.h"
#include <KMacroExpander>

#include <QTextToSpeech>

NotifyByTTS::NotifyByTTS(QObject *parent)
    : KNotificationPlugin(parent)
    , m_speech(new QTextToSpeech(this))
{
}

NotifyByTTS::~NotifyByTTS()
{
    delete m_speech;
}

void NotifyByTTS::notify(KNotification *notification, KNotifyConfig *config)
{
    if (m_speech->state() != QTextToSpeech::BackendError) {
        QString say = config->readEntry(QStringLiteral("TTS"));

        if (!say.isEmpty()) {
            // Create a hash of characters to strings to expand text into the notification text.
            QHash<QChar, QString> subst;
            subst.insert(QLatin1Char('e'), notification->eventId());
            subst.insert(QLatin1Char('a'), notification->appName());
            subst.insert(QLatin1Char('s'), notification->text());
            // subst.insert('w', QString::number((quintptr)config->winId));
            // subst.insert('i', QString::number(id));
            subst.insert(QLatin1Char('m'), notification->text());
            say = KMacroExpander::expandMacrosShellQuote(say, subst);
        }

        if (say.isEmpty()) {
            say = notification->text(); // fallback on the plain text
        }

        m_speech->say(say);

        Q_EMIT finished(notification);
    } else {
        qCDebug(LOG_KNOTIFICATIONS) << "Speech backend has an error, not speaking";
    }
}
