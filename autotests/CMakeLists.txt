include(ECMAddTests)

find_package(Qt5Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

set(KNotificationTest_SRCS knotification_test.cpp fake_notifications_server.cpp)
ecm_add_test(${KNotificationTest_SRCS} TEST_NAME "KNotificationTest" LINK_LIBRARIES Qt5::Test Qt5::DBus KF5::Notifications)
