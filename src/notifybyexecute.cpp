/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "notifybyexecute.h"

#include <QGuiApplication>
#include <QHash>

#include "debug_p.h"
#include "knotification.h"
#include <knotifyconfig.h>

#include <KProcess>
#include <KMacroExpander>

/*
 * The UI part for this backend is in plasma-workspace. Take a look at:
 *
 * kcms/notifications/ui/ApplicationConfiguration.qml
 */

NotifyByExecute::NotifyByExecute(QObject *parent)
    : KNotificationPlugin(parent)
{
}

NotifyByExecute::~NotifyByExecute()
{
}

void NotifyByExecute::notify(KNotification *notification, const KNotifyConfig &notifyConfig)
{
    const QString command = notifyConfig.readEntry(QStringLiteral("Execute"));

    if (!command.isEmpty()) {
        QHash<QChar, QString> subst;
        subst.insert(QLatin1Char('i'), QString::number(notification->id()));
        subst.insert(QLatin1Char('e'), notification->eventId());
        subst.insert(QLatin1Char('a'), notification->appName());
        subst.insert(QLatin1Char('n'), QGuiApplication::applicationDisplayName());

        subst.insert(QLatin1Char('t'), notification->title());
        subst.insert(QLatin1Char('d'), notification->text());

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
