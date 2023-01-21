include(ECMMarkAsTest)

macro(knotifications_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt${QT_MAJOR_VERSION}::DBus KF6::Notifications)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

if (TARGET Qt${QT_MAJOR_VERSION}::DBus)
    knotifications_executable_tests(
        knotificationrestrictionstest
        unitylaunchertest
        kstatusnotifieritemtest
        knotificationdbustest
    )
endif()
