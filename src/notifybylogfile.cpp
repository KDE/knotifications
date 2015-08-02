/*
   Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>

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


#include "notifybylogfile.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QString>

#include "knotifyconfig.h"
#include "knotification.h"

NotifyByLogfile::NotifyByLogfile(QObject *parent)
    : KNotificationPlugin(parent)
{
}

NotifyByLogfile::~NotifyByLogfile()
{
}

void NotifyByLogfile::notify(KNotification *notification, KNotifyConfig *config)
{
    QString file = config->readEntry("Logfile");

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
    strm << text << endl;

    // close file
    logFile.close();

    finish(notification);
}

