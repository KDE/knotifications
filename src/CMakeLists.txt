add_library(KF5Notifications)
add_library(KF5::Notifications ALIAS KF5Notifications)

ecm_create_qm_loader(KF5Notifications knotifications5_qt)

target_sources(KF5Notifications PRIVATE
  knotification.cpp
  knotificationreplyaction.cpp
  knotificationmanager.cpp
  kpassivepopup.cpp

  knotifyconfig.cpp
  knotificationplugin.cpp
  knotificationjobuidelegate.cpp
  notifybylogfile.cpp
)

if (NOT ANDROID)
    target_sources(KF5Notifications PRIVATE
        notifybyexecute.cpp
        notifybytaskbar.cpp
        kstatusnotifieritem.cpp)
endif()

if (TARGET Qt${QT_MAJOR_VERSION}::DBus)
  target_sources(KF5Notifications PRIVATE
    kstatusnotifieritemdbus_p.cpp
    knotificationrestrictions.cpp

    imageconverter.cpp #needed to marshal images for sending over dbus by NotifyByPopup
    notifybypopup.cpp
    notifybyportal.cpp
  )
endif()

if (ANDROID)
    add_subdirectory(android)
    target_sources(KF5Notifications PRIVATE
        notifybyandroid.cpp
        knotifications.qrc
    )
endif()

if (WITH_SNORETOAST)
    target_sources(KF5Notifications PRIVATE notifybysnore.cpp)
endif ()

if (APPLE)
    target_sources(KF5Notifications PRIVATE notifybymacosnotificationcenter.mm)
endif()

ecm_qt_declare_logging_category(KF5Notifications
    HEADER debug_p.h
    IDENTIFIER LOG_KNOTIFICATIONS
    CATEGORY_NAME kf.notifications
    OLD_CATEGORY_NAMES org.kde.knotifications
    DESCRIPTION "KNotifications"
    EXPORT KNOTIFICATIONS
)

if (Canberra_FOUND)
    target_sources(KF5Notifications PRIVATE
        notifybyaudio_canberra.cpp
    )
elseif (Phonon4Qt5_FOUND)
    target_sources(KF5Notifications PRIVATE
        notifybyaudio_phonon.cpp
    )
endif()

if (Qt5TextToSpeech_FOUND)
    target_sources(KF5Notifications PRIVATE
        notifybytts.cpp
    )
endif()

if (TARGET Qt${QT_MAJOR_VERSION}::DBus)
  if (dbusmenu-qt5_FOUND)
    message("dbusmenu-qt5_FOUND")
    set(HAVE_DBUSMENUQT 1)
    target_include_directories(KF5Notifications PRIVATE ${dbusmenu-qt5_INCLUDE_DIRS})
  else()
    set(HAVE_DBUSMENUQT 0)
  endif()

  set(knotifications_dbus_SRCS)
  qt_add_dbus_adaptor(knotifications_dbus_SRCS     org.kde.StatusNotifierItem.xml
                    kstatusnotifieritemdbus_p.h KStatusNotifierItemDBus)


  set(statusnotifierwatcher_xml org.kde.StatusNotifierWatcher.xml)
  qt_add_dbus_interface(knotifications_dbus_SRCS ${statusnotifierwatcher_xml} statusnotifierwatcher_interface)
  set(notifications_xml org.freedesktop.Notifications.xml)
  qt_add_dbus_interface(knotifications_dbus_SRCS ${notifications_xml} notifications_interface)
  target_sources(KF5Notifications PRIVATE ${knotifications_dbus_SRCS})
endif()

configure_file(config-knotifications.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-knotifications.h )

ecm_generate_export_header(KF5Notifications
    EXPORT_FILE_NAME knotifications_export.h
    BASE_NAME KNotifications
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.67 5.75 5.76 5.79
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF5Notifications INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KNotifications>")

target_link_libraries(KF5Notifications PUBLIC Qt${QT_MAJOR_VERSION}::Gui)
if (TARGET Qt${QT_MAJOR_VERSION}::Widgets)
    target_link_libraries(KF5Notifications PUBLIC Qt${QT_MAJOR_VERSION}::Widgets)
endif()
if (TARGET Qt${QT_MAJOR_VERSION}::DBus)
   target_link_libraries(KF5Notifications PUBLIC Qt${QT_MAJOR_VERSION}::DBus)
endif()
target_link_libraries(KF5Notifications PRIVATE
   KF5::CoreAddons
   KF5::ConfigCore
)

if (TARGET KF5::WindowSystem)
    target_link_libraries(KF5Notifications PRIVATE KF5::WindowSystem)
endif()

if (TARGET SnoreToast::SnoreToastActions)
  target_link_libraries(KF5Notifications PRIVATE Qt${QT_MAJOR_VERSION}::Network SnoreToast::SnoreToastActions)
endif ()

if (Phonon4Qt5_FOUND)
    target_link_libraries(KF5Notifications PRIVATE
        Phonon::phonon4qt5)
endif()

if (Canberra_FOUND)
    target_link_libraries(KF5Notifications PRIVATE
        Canberra::Canberra)
endif()

if (Qt5TextToSpeech_FOUND)
    target_link_libraries(KF5Notifications PRIVATE Qt5::TextToSpeech)
endif()

if(X11_FOUND)
   target_link_libraries(KF5Notifications PRIVATE ${X11_X11_LIB})
    if (TARGET Qt5::X11Extras)
        target_link_libraries(KF5Notifications PRIVATE Qt5::X11Extras)
    elseif (TARGET Qt6::Gui)
        target_link_libraries(KF5Notifications PRIVATE Qt6::GuiPrivate) # qtx11extras_p.h
    endif()
endif()

if(APPLE)
  target_link_libraries(KF5Notifications PRIVATE Qt5::MacExtras "-framework Foundation" "-framework AppKit")
endif()

if(X11_XTest_FOUND)
  target_link_libraries(KF5Notifications PRIVATE ${X11_XTest_LIB})
endif()

if(HAVE_DBUSMENUQT)
  target_link_libraries(KF5Notifications PRIVATE dbusmenu-qt5)
endif()

if (ANDROID)
    target_link_libraries(KF5Notifications PRIVATE Qt5::AndroidExtras)
endif()

set_target_properties(KF5Notifications PROPERTIES VERSION   ${KNOTIFICATIONS_VERSION}
                                                  SOVERSION ${KNOTIFICATIONS_SOVERSION}
                                                  EXPORT_NAME Notifications
)

ecm_generate_headers(KNotifications_HEADERS
  HEADER_NAMES
  KNotification
  KPassivePopup
  KStatusNotifierItem
  KNotificationJobUiDelegate
  KNotificationReplyAction
  KNotificationRestrictions
  KNotificationPlugin
  KNotifyConfig

  REQUIRED_HEADERS KNotifications_HEADERS
)

install(TARGETS KF5Notifications EXPORT KF5NotificationsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/knotifications_export.h
  ${KNotifications_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KNotifications COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KNOTIFICATIONS
    FILE knotifications.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF5Notifications_QCH
        NAME KNotifications
        BASE_NAME KF5Notifications
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KNotifications_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt5Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KNOTIFICATIONS_EXPORT
            KNOTIFICATIONS_DEPRECATED
            KNOTIFICATIONS_DEPRECATED_EXPORT
            "KNOTIFICATIONS_DEPRECATED_VERSION(x, y, t)"
            "KNOTIFICATIONS_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

if (TARGET Qt${QT_MAJOR_VERSION}::DBus)
    install(FILES
        org.kde.StatusNotifierItem.xml
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
        RENAME kf5_org.kde.StatusNotifierItem.xml)

    install(FILES
        org.kde.StatusNotifierWatcher.xml
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
        RENAME kf5_org.kde.StatusNotifierWatcher.xml)
endif()

if (NOT ANDROID)
    install(FILES
        knotificationplugin.desktop
        DESTINATION ${KDE_INSTALL_KSERVICETYPESDIR})
endif()

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KNotifications LIB_NAME KF5Notifications DEPS "widgets" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KNotifications)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

if (ANDROID)
    install(FILES KF5Notifications-android-dependencies.xml
        DESTINATION ${KDE_INSTALL_LIBDIR}
        RENAME KF5Notifications_${CMAKE_ANDROID_ARCH_ABI}-android-dependencies.xml
    )
endif()

if (TARGET Qt${QT_MAJOR_VERSION}::Qml)
    add_subdirectory(qml)
endif()
