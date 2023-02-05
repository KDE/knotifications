cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "5.103.0") # handled by release scripts
set(KF_DEP_VERSION "5.103.0") # handled by release scripts
project(KNotifications VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 5.103.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(KDEGitCommitHooks)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMPoQmTools)
include(ECMAddQch)
include(ECMGenerateExportHeader)
include(ECMDeprecationSettings)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_setup_version(PROJECT
  VARIABLE_PREFIX KNOTIFICATIONS
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/knotifications_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5NotificationsConfigVersion.cmake"

                        SOVERSION 5)

# Dependencies
set(REQUIRED_QT_VERSION 5.15.2)
set(CMAKE_AUTORCC TRUE)

if (ANDROID)
    find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Gui)
    if (QT_MAJOR_VERSION EQUAL "5")
        find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED AndroidExtras)
    endif()
    find_package(Gradle REQUIRED)
    find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} QUIET OPTIONAL_COMPONENTS Widgets)
    if (NOT TARGET Qt${QT_MAJOR_VERSION}::Widgets AND EXCLUDE_DEPRECATED_BEFORE_AND_AT EQUAL 0)
        set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 5.79.0)
    endif()
else()
    find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)
endif()

if (NOT ANDROID AND NOT WIN32)
    find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
endif()

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} QUIET OPTIONAL_COMPONENTS Qml TextToSpeech)

find_package(Qt${QT_MAJOR_VERSION}TextToSpeech ${REQUIRED_QT_VERSION} QUIET CONFIG)
set_package_properties(Qt${QT_MAJOR_VERSION}TextToSpeech PROPERTIES
    DESCRIPTION "Qt text to speech module"
    TYPE OPTIONAL
    PURPOSE "Required to build text to speech notification support")
if (TARGET Qt${QT_MAJOR_VERSION}::TextToSpeech)
    add_definitions(-DHAVE_SPEECH)
endif()

if (NOT APPLE AND NOT WIN32)
    option(WITHOUT_X11 "Build without X11 support (skips finding X11)." OFF)
    if(NOT WITHOUT_X11)
        find_package(X11)
        set(HAVE_X11 ${X11_FOUND})
        set(HAVE_XTEST ${X11_XTest_FOUND})
        if(X11_FOUND AND QT_MAJOR_VERSION STREQUAL "5")
            find_package(Qt5X11Extras ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED)
        endif()
    endif()
endif()

if(APPLE)
   find_package(Qt5MacExtras ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
endif()
if (WIN32)
    find_package(LibSnoreToast)
    find_package(Qt${QT_MAJOR_VERSION}Network REQUIRED)
    
    include(CMakeDependentOption)
    cmake_dependent_option(WITH_SNORETOAST "for the Windows Toast Notifications" ON
                           "Qt${QT_MAJOR_VERSION}Network_FOUND;LibSnoreToast_FOUND" OFF
    )    
endif()

set(HAVE_KWINDOWSYSTEM FALSE)
if(NOT ANDROID)
    find_package(KF5WindowSystem ${KF_DEP_VERSION} REQUIRED)
    set(HAVE_KWINDOWSYSTEM TRUE)
endif()

find_package(KF5Config ${KF_DEP_VERSION} REQUIRED)
find_package(KF5CoreAddons ${KF_DEP_VERSION} REQUIRED)

if (NOT ANDROID AND NOT WIN32 OR (WIN32 AND NOT WITH_SNORETOAST))
    find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
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
# dbusmenu-qt5 needs to be built with Qt6 first
if (TARGET Qt5::DBus)
    find_package(dbusmenu-qt5 CONFIG)
    set_package_properties(dbusmenu-qt5 PROPERTIES DESCRIPTION "DBusMenuQt"
                            URL "https://launchpad.net/libdbusmenu-qt" TYPE OPTIONAL
                            PURPOSE "Support for notification area menus via the DBusMenu protocol")

    set(HAVE_DBUS TRUE)
endif()
set(HAVE_WIDGETS FALSE)
if (TARGET Qt${QT_MAJOR_VERSION}::Widgets)
    set(HAVE_WIDGETS TRUE)
endif()

remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

ecm_install_po_files_as_qm(poqm)

if (NOT APPLE) # QtMac::setBadgeLabelText is deprecated
ecm_set_disabled_deprecation_versions(QT 5.15.2)
endif()
ecm_set_disabled_deprecation_versions(KF 5.97)

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
    add_subdirectory(examples)
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
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KNotifications COMPONENT Devel
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
