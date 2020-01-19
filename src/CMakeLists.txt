if (Phonon4Qt5_FOUND)
    include_directories(${PHONON_INCLUDE_DIR})
endif()
if (CANBERRA_FOUND)
    include_directories(${CANBERRA_INCLUDE_DIRS})
endif()

ecm_create_qm_loader(knotifications_QM_LOADER knotifications5_qt)

set(knotifications_SRCS

  knotification.cpp
  knotificationmanager.cpp
  kpassivepopup.cpp

  knotifyconfig.cpp
  knotificationplugin.cpp
  notifybyexecute.cpp
  notifybylogfile.cpp
  notifybytaskbar.cpp
  ${knotifications_QM_LOADER}
)

if (TARGET Qt5::DBus)
  list(APPEND knotifications_SRCS
    kstatusnotifieritem.cpp
    kstatusnotifieritemdbus_p.cpp
    knotificationrestrictions.cpp

    imageconverter.cpp #needed to marshal images for sending over dbus by NotifyByPopup
    notifybypopup.cpp
    notifybyportal.cpp
  )
endif()

if (ANDROID)
    add_subdirectory(android)
    list(APPEND knotifications_SRCS notifybyandroid.cpp knotifications.qrc)
endif()

if (WIN32)
  list(APPEND knotifications_SRCS notifybysnore.cpp)
endif ()

if (APPLE)
  list(APPEND knotifications_SRCS notifybymacosnotificationcenter.mm)
endif()

ecm_qt_declare_logging_category(knotifications_SRCS HEADER debug_p.h IDENTIFIER LOG_KNOTIFICATIONS CATEGORY_NAME org.kde.knotifications)

if (Canberra_FOUND)
  set(knotifications_SRCS ${knotifications_SRCS}
  notifybyaudio_canberra.cpp)
elseif (Phonon4Qt5_FOUND)
  set(knotifications_SRCS ${knotifications_SRCS}
  notifybyaudio_phonon.cpp)
endif()

if (Qt5TextToSpeech_FOUND)
  set(knotifications_SRCS ${knotifications_SRCS}
    notifybytts.cpp)
endif()

if (TARGET Qt5::DBus)
  if (dbusmenu-qt5_FOUND)
    message("dbusmenu-qt5_FOUND")
    set(HAVE_DBUSMENUQT 1)
    include_directories(${dbusmenu-qt5_INCLUDE_DIRS})
  else()
    set(HAVE_DBUSMENUQT 0)
  endif()

  qt5_add_dbus_adaptor(knotifications_SRCS     org.kde.StatusNotifierItem.xml
                    kstatusnotifieritemdbus_p.h KStatusNotifierItemDBus)


  set(statusnotifierwatcher_xml org.kde.StatusNotifierWatcher.xml)
  qt5_add_dbus_interface(knotifications_SRCS ${statusnotifierwatcher_xml} statusnotifierwatcher_interface)
  set(notifications_xml org.freedesktop.Notifications.xml)
  qt5_add_dbus_interface(knotifications_SRCS ${notifications_xml} notifications_interface)
endif()

configure_file(config-knotifications.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-knotifications.h )

add_library(KF5Notifications ${knotifications_SRCS})
add_library(KF5::Notifications ALIAS KF5Notifications)

ecm_generate_export_header(KF5Notifications
    EXPORT_FILE_NAME knotifications_export.h
    BASE_NAME KNotifications
    GROUP_BASE_NAME KF
    VERSION ${KF5_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.67
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF5Notifications INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KNotifications>")

target_link_libraries(KF5Notifications PUBLIC
   Qt5::Widgets
)
if (TARGET Qt5::DBus)
   target_link_libraries(KF5Notifications PUBLIC Qt5::DBus)
endif()
target_link_libraries(KF5Notifications PRIVATE
   KF5::CoreAddons
   KF5::ConfigCore
)

if (TARGET KF5::WindowSystem)
    target_link_libraries(KF5Notifications PRIVATE KF5::WindowSystem)
endif()

if (TARGET SnoreToast::SnoreToastActions)
  target_link_libraries(KF5Notifications PRIVATE Qt5::Network SnoreToast::SnoreToastActions)
endif ()

if (Phonon4Qt5_FOUND)
    target_link_libraries(KF5Notifications PRIVATE
        ${PHONON_LIBRARIES})
endif()

if (Canberra_FOUND)
    target_link_libraries(KF5Notifications PRIVATE
        Canberra::Canberra)
endif()

if (Qt5TextToSpeech_FOUND)
    target_link_libraries(KF5Notifications PRIVATE Qt5::TextToSpeech)
endif()

if(X11_FOUND)
   target_link_libraries(KF5Notifications PRIVATE ${X11_X11_LIB} Qt5::X11Extras)
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

set_target_properties(KF5Notifications PROPERTIES VERSION   ${KNOTIFICATIONS_VERSION_STRING}
                                                  SOVERSION ${KNOTIFICATIONS_SOVERSION}
                                                  EXPORT_NAME Notifications
)

ecm_generate_headers(KNotifications_HEADERS
  HEADER_NAMES
  KNotification
  KPassivePopup
  KStatusNotifierItem
  KNotificationRestrictions
  KNotificationPlugin
  KNotifyConfig

  REQUIRED_HEADERS KNotifications_HEADERS
)

install(TARGETS KF5Notifications EXPORT KF5NotificationsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/knotifications_export.h
  ${KNotifications_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNotifications COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF5Notifications_QCH
        NAME KNotifications
        BASE_NAME KF5Notifications
        VERSION ${KF5_VERSION}
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
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

if (TARGET Qt5::DBus)
    install(FILES
        org.kde.StatusNotifierItem.xml
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
        RENAME kf5_org.kde.StatusNotifierItem.xml)

    install(FILES
        org.kde.StatusNotifierWatcher.xml
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
        RENAME kf5_org.kde.StatusNotifierWatcher.xml)
endif()

install(FILES
    knotificationplugin.desktop
    DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KNotifications LIB_NAME KF5Notifications DEPS "widgets" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KNotifications)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

if (ANDROID)
    if (Qt5Core_VERSION VERSION_LESS 5.14.0)
        install(FILES KF5Notifications-android-dependencies.xml DESTINATION ${KDE_INSTALL_LIBDIR})
    else()
        install(FILES KF5Notifications-android-dependencies.xml DESTINATION ${KDE_INSTALL_LIBDIR} RENAME KF5Notifications_${CMAKE_ANDROID_ARCH_ABI}-android-dependencies.xml)
    endif()
endif()
