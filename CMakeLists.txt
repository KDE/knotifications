cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "5.240.0") # handled by release scripts
set(KF_DEP_VERSION "5.240.0") # handled by release scripts
project(KNotifications VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 5.240.0  NO_MODULE)
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

set(knotifications_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/knotifications_version.h")
ecm_setup_version(PROJECT
  VARIABLE_PREFIX KNOTIFICATIONS
  VERSION_HEADER "${knotifications_version_header}"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6NotificationsConfigVersion.cmake"

                        SOVERSION 6)

# Dependencies
set(REQUIRED_QT_VERSION 6.4.0)
set(CMAKE_AUTORCC TRUE)

if (ANDROID)
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Gui)
    find_package(Gradle REQUIRED)
    find_package(Qt6 ${REQUIRED_QT_VERSION} QUIET OPTIONAL_COMPONENTS Widgets)
else()
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)
endif()

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE)
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
endif()

find_package(Qt6 ${REQUIRED_QT_VERSION} QUIET OPTIONAL_COMPONENTS Qml)

if (NOT APPLE AND NOT WIN32)
    option(WITHOUT_X11 "Build without X11 support (skips finding X11)." OFF)
    if(NOT WITHOUT_X11)
        find_package(X11)
        set(HAVE_X11 ${X11_FOUND})
        set(HAVE_XTEST ${X11_XTest_FOUND})
    endif()
endif()

if (WIN32)
    find_package(LibSnoreToast)
    find_package(Qt6Network REQUIRED)
    
    include(CMakeDependentOption)
    cmake_dependent_option(WITH_SNORETOAST "for the Windows Toast Notifications" ON
                           "Qt6Network_FOUND;LibSnoreToast_FOUND" OFF
    )    
endif()

set(HAVE_KWINDOWSYSTEM FALSE)
if(NOT ANDROID)
    find_package(KF6WindowSystem ${KF_DEP_VERSION} REQUIRED)
    set(HAVE_KWINDOWSYSTEM TRUE)
endif()

find_package(KF6Config ${KF_DEP_VERSION} REQUIRED)
find_package(KF6CoreAddons ${KF_DEP_VERSION} REQUIRED)

if (NOT APPLE AND NOT ANDROID AND NOT WIN32 OR (WIN32 AND NOT WITH_SNORETOAST))
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
    find_package(Canberra REQUIRED)
    set_package_properties(Canberra PROPERTIES
        PURPOSE "Needed to build audio notification support")
    if (TARGET Canberra::Canberra)
        add_definitions(-DHAVE_CANBERRA)
    endif()
endif()

set(HAVE_DBUS FALSE)
# dbusmenu-qt5 needs to be built with Qt6 first
if (TARGET Qt6::DBus)
    set(HAVE_DBUS TRUE)
endif()
set(HAVE_WIDGETS FALSE)
if (TARGET Qt6::Widgets)
    set(HAVE_WIDGETS TRUE)
endif()

remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

ecm_install_po_files_as_qm(poqm)

ecm_set_disabled_deprecation_versions(
    QT 6.4
    KF 5.240
)

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
    add_subdirectory(examples)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Notifications")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Notifications_QCH
        FILE KF6NotificationsQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6NotificationsQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6NotificationsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NotificationsConfig.cmake"
  PATH_VARS KDE_INSTALL_DBUSINTERFACEDIR
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NotificationsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6NotificationsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6NotificationsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6NotificationsTargets.cmake NAMESPACE KF6:: )

install(FILES
   ${knotifications_version_header}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KNotifications COMPONENT Devel
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
