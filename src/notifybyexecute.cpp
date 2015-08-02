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

#include "notifybyexecute.h"

#include <QHash>
#include <QWidget>

#include <KProcess>
#include <knotifyconfig.h>
#include "knotification.h"
#include "debug_p.h"

#include <kmacroexpander.h>

NotifyByExecute::NotifyByExecute(QObject *parent)
    : KNotificationPlugin(parent)
{
}


NotifyByExecute::~NotifyByExecute()
{
}

void NotifyByExecute::notify(KNotification *notification, KNotifyConfig *config)
{
    QString command = config->readEntry(QStringLiteral("Execute"));

    if (!command.isEmpty()) {
        QHash<QChar,QString> subst;
        subst.insert('e', notification->eventId());
        subst.insert('a', notification->appName());
        subst.insert('s', notification->text());
        if (notification->widget()) {
            subst.insert('w', QString::number(notification->widget()->topLevelWidget()->winId()));
        } else {
            subst.insert('w', QStringLiteral("0"));
        }
        subst.insert('i', QString::number(notification->id()));

        QString execLine = KMacroExpander::expandMacrosShellQuote(command, subst);
        if (execLine.isEmpty()) {
            execLine = command; // fallback
        }

        KProcess proc;
        proc.setShellCommand(execLine.trimmed());
        if (!proc.startDetached()) {
            qCDebug(LOG_KNOTIFICATIONS) << "KProcess returned an error while trying to execute this command:" << execLine;
        }
    }

    finish(notification);
}

