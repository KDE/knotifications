/*
    SPDX-FileCopyrightText: 2006 Aaron J. Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <knotificationrestrictions.h>

#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QLabel *mainWidget = new QLabel();
    mainWidget->setText(QStringLiteral("You should see on console debug outpout for KNotificationRestrictions"));

    KNotificationRestrictions knr(KNotificationRestrictions::ScreenSaver, nullptr);

    mainWidget->show();
    return app.exec();
}
