# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(knotificationqmlplugin)
target_sources(knotificationqmlplugin PRIVATE
    knotificationqmlplugin.cpp
)
target_link_libraries(knotificationqmlplugin PRIVATE
    KF5Notifications
    Qt${QT_MAJOR_VERSION}::Qml
)

if(MINGW)
    set_target_properties(knotificationqmlplugin PROPERTIES PREFIX "")
endif()

install(TARGETS knotificationqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/notification)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/notification)
