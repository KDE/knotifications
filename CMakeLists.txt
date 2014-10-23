cmake_minimum_required(VERSION 2.8.12)

project(KNotifications)

# ECM setup
find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

include(ECMPoQmTools)

set(KF5_VERSION "5.4.0") # handled by release scripts
set(KF5_DEP_VERSION "5.3.0") # handled by release scripts

ecm_setup_version(${KF5_VERSION}
  VARIABLE_PREFIX KNOTIFICATIONS
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/knotifications_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5NotificationsConfigVersion.cmake"

                        SOVERSION 5)

# Dependencies
set(REQUIRED_QT_VERSION "5.2.0")

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets DBus)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

if (NOT APPLE)
    find_package(X11)
endif()

set(HAVE_X11 ${X11_FOUND})
set(HAVE_XTEST ${X11_XTest_FOUND})
if(X11_FOUND)
   find_package(Qt5X11Extras 5.2.0 REQUIRED NO_MODULE)
endif()

find_package(KF5WindowSystem ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Service ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Config ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5IconThemes ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Codecs ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5CoreAddons ${KF5_DEP_VERSION} REQUIRED)

find_package(Phonon4Qt5 4.6.60 REQUIRED NO_MODULE)
set_package_properties(Phonon4Qt5 PROPERTIES
   DESCRIPTION "Qt-based audio library"
   TYPE REQUIRED)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ecm_install_po_files_as_qm(po)
endif()

add_subdirectory(src)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Notifications")

include(ECMPackageConfigHelpers)

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5NotificationsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NotificationsConfig.cmake"
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
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR} COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
