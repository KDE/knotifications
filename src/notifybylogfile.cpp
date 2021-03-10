/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "notifybylogfile.h"

#include <QDateTime>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QUrl>

#include "knotification.h"
#include "knotifyconfig.h"

NotifyByLogfile::NotifyByLogfile(QObject *parent)
    : KNotificationPlugin(parent)
{
}

NotifyByLogfile::~NotifyByLogfile()
{
}

void NotifyByLogfile::notify(KNotification *notification, KNotifyConfig *config)
{
    QString file = config->readEntry(QStringLiteral("Logfile"));

    if (file.isEmpty()) {
        finish(notification);
        return;
    }

    // open file in append mode
    QFile logFile(QUrl(file).path());

    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        finish(notification);
        return;
    }

    QString text = notification->text();
    if (text.isEmpty()) {
        text = config->readEntry(QStringLiteral("Name"));
    }
    // append msg
    QTextStream strm(&logFile);
    strm << "- KNotify " << QDateTime::currentDateTime().toString() << ": ";
    strm << text << QLatin1Char('\n');

    // close file
    logFile.close();

    finish(notification);
}
