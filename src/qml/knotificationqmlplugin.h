/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNOTIFICATIONQMLPLUGIN_H
#define KNOTIFICATIONQMLPLUGIN_H

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class KNotificationQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void registerTypes(const char *uri) override;
};

#endif // KNOTIFICATIONQMLPLUGIN_H
