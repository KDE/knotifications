/* This file is part of the KDE libraries
   Copyright 2009 by Marco Martin <notmart@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kstatusnotifieritemtest.h"

#include "kstatusnotifieritem.h"
#include <QDateTime>
#include <QLabel>
#include <QMenu>
#include <QMovie>
#include <QApplication>

#include <qcommandlineparser.h>

#include <qdebug.h>

KStatusNotifierItemTest::KStatusNotifierItemTest(QObject *parent, KStatusNotifierItem *tray)
    : QObject(parent)
{
    QMenu *menu = tray->contextMenu();
    m_tray = tray;

    QAction *needsAttention = new QAction(QStringLiteral("Set needs attention"), menu);
    QAction *active = new QAction(QStringLiteral("Set active"), menu);
    QAction *passive = new QAction(QStringLiteral("Set passive"), menu);

    menu->addAction(needsAttention);
    menu->addAction(active);
    menu->addAction(passive);

    connect(needsAttention, SIGNAL(triggered()), this, SLOT(setNeedsAttention()));
    connect(active, SIGNAL(triggered()), this, SLOT(setActive()));
    connect(passive, SIGNAL(triggered()), this, SLOT(setPassive()));
}

void KStatusNotifierItemTest::setNeedsAttention()
{
    qDebug() << "Asking for attention";
    m_tray->showMessage(QStringLiteral("message test"), QStringLiteral("Test of the new systemtray notifications wrapper"), QStringLiteral("konqueror"), 3000);
    m_tray->setStatus(KStatusNotifierItem::NeedsAttention);
}

void KStatusNotifierItemTest::setActive()
{
    qDebug() << "Systray icon in active state";
    m_tray->setStatus(KStatusNotifierItem::Active);
}

void KStatusNotifierItemTest::setPassive()
{
    qDebug() << "Systray icon in passive state";
    m_tray->setStatus(KStatusNotifierItem::Passive);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    int ksniCount;
    QString iconName;
    {
        QCommandLineParser parser;
        parser.setApplicationDescription(QCoreApplication::translate("main", "KStatusNotifierItemtest"));
        parser.addHelpOption();
        parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("active-icon"), QCoreApplication::translate("main", "Name of active icon"), QStringLiteral("name"), QStringLiteral("konqueror")));
        parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("ksni-count"), QCoreApplication::translate("main", "How many instances of KStatusNotifierItem to create"), QStringLiteral("count"), QStringLiteral("1")));
        parser.process(app);

        if (parser.positionalArguments().count() != 0) {
            parser.showHelp();
            return (1);
        }
        ksniCount = parser.value(QStringLiteral("ksni-count")).toInt();
        iconName = parser.value(QStringLiteral("active-icon"));
    }

    QLabel *l = new QLabel(QStringLiteral("System Tray Main Window"), 0L);
    for (int x = 0; x < ksniCount; ++x) {
        KStatusNotifierItem *tray = new KStatusNotifierItem(l);

        new KStatusNotifierItemTest(0, tray);

        tray->setTitle(QStringLiteral("DBus System tray test"));
        tray->setIconByName(iconName);
        //tray->setIconByPixmap(QIcon::fromTheme("konqueror"));
        //tray->setAttentionIconByName("kmail");
        tray->setOverlayIconByName(QStringLiteral("emblem-important"));
        tray->setStatus(KStatusNotifierItem::Active);

        tray->setToolTipIconByName(QStringLiteral("konqueror"));
        tray->setToolTipTitle(QStringLiteral("DBus System tray test"));
        tray->setToolTipSubTitle(QStringLiteral("This is a test of the new systemtray specification"));

        tray->setToolTip(QStringLiteral("konqueror"), QStringLiteral("DBus System tray test #%1").arg(x + 1), QStringLiteral("This is a test of the new systemtray specification"));

        tray->showMessage(QStringLiteral("message test"), QStringLiteral("Test of the new systemtray notifications wrapper"), QStringLiteral("konqueror"), 3000);
        //tray->setStandardActionsEnabled(false);
    }

    return app.exec();
}

