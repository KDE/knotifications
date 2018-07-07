cmake_minimum_required(VERSION 3.0)

set(KF5_VERSION "5.48.0") # handled by release scripts
set(KF5_DEP_VERSION "5.48.0") # handled by release scripts
project(KNotifications VERSION ${KF5_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 5.48.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})


include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMPoQmTools)
include(ECMAddQch)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_setup_version(PROJECT
  VARIABLE_PREFIX KNOTIFICATIONS
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/knotifications_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5NotificationsConfigVersion.cmake"

                        SOVERSION 5)

# Dependencies
set(REQUIRED_QT_VERSION 5.8.0)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets DBus)
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

find_package(KF5WindowSystem ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Config ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Codecs ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5CoreAddons ${KF5_DEP_VERSION} REQUIRED)

find_package(Phonon4Qt5 4.6.60 REQUIRED NO_MODULE)
set_package_properties(Phonon4Qt5 PROPERTIES
   DESCRIPTION "Qt-based audio library"
   TYPE REQUIRED
   PURPOSE "Required to build audio notification support")
if (Phonon4Qt5_FOUND)
  add_definitions(-DHAVE_PHONON4QT5)
endif()

remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ecm_install_po_files_as_qm(po)
endif()

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(autotests)

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

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
