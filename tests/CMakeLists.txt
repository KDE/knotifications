
include(ECMMarkAsTest)

find_package(ECM 0.0.9  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

macro(knotifications_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Test Qt5::DBus KF5::Notifications)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

knotifications_executable_tests(
    knotificationrestrictionstest
    kpassivepopuptest
    kstatusnotifieritemtest
    knotificationdbustest
)

target_link_libraries(kpassivepopuptest KF5::WindowSystem)
