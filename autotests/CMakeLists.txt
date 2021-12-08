include(ECMAddTests)

find_package(Qt${QT_MAJOR_VERSION}Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT TARGET Qt${QT_MAJOR_VERSION}::Test)
    message(STATUS "Qt${QT_MAJOR_VERSION}Test not found, autotests will not be built.")
    return()
endif()

#if (TARGET Qt${QT_MAJOR_VERSION}::DBus)
#    set(KNotificationTest_SRCS knotification_test.cpp fake_notifications_server.cpp)
#    ecm_add_test(${KNotificationTest_SRCS} TEST_NAME "KNotificationTest" LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test Qt${QT_MAJOR_VERSION}::DBus KF5::Notifications)
#endif()
