/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "notifybyexecute.h"

#include <QGuiApplication>
#include <QHash>
#include <QWidget>

#include "debug_p.h"
#include "knotification.h"
#include <KProcess>
#include <knotifyconfig.h>

#include <KMacroExpander>

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
        QHash<QChar, QString> subst;
        subst.insert(QLatin1Char('e'), notification->eventId());
        subst.insert(QLatin1Char('a'), notification->appName());
        subst.insert(QLatin1Char('s'), notification->text());
        if (notification->widget()) {
            subst.insert(QLatin1Char('w'), QString::number(notification->widget()->topLevelWidget()->winId()));
            subst.insert(QLatin1Char('t'), notification->widget()->topLevelWidget()->windowTitle());
        } else {
            subst.insert(QLatin1Char('w'), QStringLiteral("0"));
        }
        subst.insert(QLatin1Char('i'), QString::number(notification->id()));
        subst.insert(QLatin1Char('d'), QGuiApplication::applicationDisplayName());

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
