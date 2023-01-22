find_package(Qt6 COMPONENTS Quick)

add_executable(notificationtester notificationtester.cpp resources.qrc)

target_link_libraries(notificationtester Qt6::Quick KF6::Notifications)
