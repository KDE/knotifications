cmake_minimum_required(VERSION 3.5)

set(KF5_VERSION "5.67.0") # handled by release scripts
set(KF5_DEP_VERSION "5.66.0") # handled by release scripts
project(KNotifications VERSION ${KF5_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 5.66.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)


include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMPoQmTools)
include(ECMAddQch)
include(ECMGenerateExportHeader)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_setup_version(PROJECT
  VARIABLE_PREFIX KNOTIFICATIONS
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/knotifications_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5NotificationsConfigVersion.cmake"

                        SOVERSION 5)

# Dependencies
set(REQUIRED_QT_VERSION 5.12.0)
set(CMAKE_AUTORCC TRUE)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)

if (NOT ANDROID AND NOT WIN32)
    find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
elseif (ANDROID)
    find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED AndroidExtras)
    find_package(Gradle REQUIRED)
endif()

find_package(Qt5 ${REQUIRED_QT_VERSION} QUIET OPTIONAL_COMPONENTS TextToSpeech)
set_package_properties(Qt5TextToSpeech PROPERTIES
   DESCRIPTION "Qt text to speech module"
   TYPE OPTIONAL
   PURPOSE "Required to build text to speech notification support")
if (Qt5TextToSpeech_FOUND)
  add_definitions(-DHAVE_SPEECH)
endif()

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

if (NOT APPLE AND NOT WIN32)
    find_package(X11)
endif()

set(HAVE_X11 ${X11_FOUND})
set(HAVE_XTEST ${X11_XTest_FOUND})
if(X11_FOUND)
   find_package(Qt5X11Extras ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
endif()
if(APPLE)
   find_package(Qt5MacExtras ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
endif()
if (WIN32)
    find_package(LibSnoreToast REQUIRED)
    set_package_properties(LibSnoreToast PROPERTIES TYPE REQUIRED
                            PURPOSE     "for the Windows Toast Notifications"
                            DESCRIPTION "A command line application, capable of creating
                                        Windows Toast notifications on Windows (>=)8 "
    )
    find_package(Qt5Network REQUIRED)
endif()

set(HAVE_KWINDOWSYSTEM FALSE)
if(NOT ANDROID AND NOT WIN32)
    find_package(KF5WindowSystem ${KF5_DEP_VERSION} REQUIRED)
    set(HAVE_KWINDOWSYSTEM TRUE)
endif()

find_package(KF5Config ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5CoreAddons ${KF5_DEP_VERSION} REQUIRED)

if (NOT WIN32 AND NOT ANDROID)
    find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
    find_package(Canberra)
    set_package_properties(Canberra PROPERTIES
        PURPOSE "Needed to build audio notification support"
        TYPE OPTIONAL)
    if (Canberra_FOUND)
        add_definitions(-DHAVE_CANBERRA)
    else()
        # This is REQUIRED since you cannot tell CMake "either one of those two optional ones are required"
        find_package(Phonon4Qt5 4.6.60 NO_MODULE REQUIRED)
        set_package_properties(Phonon4Qt5 PROPERTIES
            DESCRIPTION "Qt-based audio library"
            PURPOSE "Needed to build audio notification support when Canberra isn't available")
        add_definitions(-DHAVE_PHONON4QT5)
    endif()
endif()

set(HAVE_DBUS FALSE)
if (TARGET Qt5::DBus)
    find_package(dbusmenu-qt5 CONFIG)
    set_package_properties(dbusmenu-qt5 PROPERTIES DESCRIPTION "DBusMenuQt"
                            URL "https://launchpad.net/libdbusmenu-qt" TYPE OPTIONAL
                            PURPOSE "Support for notification area menus via the DBusMenu protocol")

    set(HAVE_DBUS TRUE)
endif()

remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
#add_definitions(-DQT_NO_FOREACH)
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ecm_install_po_files_as_qm(po)
endif()
if (NOT APPLE) # QtMac::setBadgeLabelText is deprecated
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)
endif()
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054200)
add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Notifications")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5Notifications_QCH
        FILE KF5NotificationsQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5NotificationsQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5NotificationsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NotificationsConfig.cmake"
  PATH_VARS KDE_INSTALL_DBUSINTERFACEDIR
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NotificationsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NotificationsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5NotificationsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5NotificationsTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/knotifications_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)
install(FILES knotifications.categories  DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})


feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
