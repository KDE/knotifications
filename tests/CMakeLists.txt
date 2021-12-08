include(ECMMarkAsTest)

macro(knotifications_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt${QT_MAJOR_VERSION}::DBus KF5::Notifications)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

if (TARGET Qt${QT_MAJOR_VERSION}::DBus)
    if(NOT EXCLUDE_DEPRECATED_BEFORE_AND_AT STREQUAL "CURRENT" AND
       EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.79.0)
        knotifications_executable_tests(
            kpassivepopuptest
        )
        target_link_libraries(kpassivepopuptest KF5::WindowSystem)
    endif()
    knotifications_executable_tests(
        knotificationrestrictionstest
        unitylaunchertest
        kstatusnotifieritemtest
        knotificationdbustest
    )
endif()
