find_package(Qt5 COMPONENTS Quick)

add_executable(notificationtester notificationtester.cpp resources.qrc)

target_link_libraries(notificationtester Qt5::Quick KF5::Notifications)
