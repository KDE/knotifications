find_package(Qt${QT_MAJOR_VERSION} COMPONENTS Quick)

add_executable(notificationtester notificationtester.cpp resources.qrc)

target_link_libraries(notificationtester Qt${QT_MAJOR_VERSION}::Quick KF5::Notifications)
