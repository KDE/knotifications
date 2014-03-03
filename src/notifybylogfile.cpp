/*
   Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>


   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */


#include "notifybylogfile.h"

#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QString>

#include "knotifyconfig.h"
#include "knotification.h"

NotifyByLogfile::NotifyByLogfile(QObject *parent)
    : KNotifyPlugin(parent)
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

#include "notifybylogfile.moc"
