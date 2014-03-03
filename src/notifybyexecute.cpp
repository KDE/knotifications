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

#include "notifybyexecute.h"

#include <QHash>
#include <QDebug>
#include <QWidget>

#include <KProcess>
#include <knotifyconfig.h>
#include "knotification.h"

#include <kmacroexpander.h>

NotifyByExecute::NotifyByExecute(QObject *parent)
    : KNotifyPlugin(parent)
{
}


NotifyByExecute::~NotifyByExecute()
{
}

void NotifyByExecute::notify(KNotification *notification, KNotifyConfig *config)
{
    QString command = config->readEntry(QStringLiteral("Execute"));

    qDebug() << command;

    if (!command.isEmpty()) {
        QHash<QChar,QString> subst;
        subst.insert('e', notification->eventId());
        subst.insert('a', notification->appName());
        subst.insert('s', notification->text());
        subst.insert('w', QString::number(notification->widget()->topLevelWidget()->winId()));
        subst.insert('i', QString::number(notification->id()));

        QString execLine = KMacroExpander::expandMacrosShellQuote(command, subst);
        if (execLine.isEmpty()) {
            execLine = command; // fallback
        }

        KProcess proc;
        proc.setShellCommand(execLine.trimmed());
        if (!proc.startDetached()) {
            qDebug() << "KNotify: Could not start process!";
        }
    }

    finish(notification);
}

#include "notifybyexecute.moc"
