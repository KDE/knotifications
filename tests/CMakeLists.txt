include(ECMMarkAsTest)

macro(knotifications_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::DBus KF5::Notifications)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

if (TARGET Qt5::DBus)
    knotifications_executable_tests(
        knotificationrestrictionstest
        kpassivepopuptest
        unitylaunchertest
        kstatusnotifieritemtest
        knotificationdbustest
    )

    target_link_libraries(kpassivepopuptest KF5::WindowSystem)
endif()
