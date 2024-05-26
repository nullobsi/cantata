add_library(KF6Notifications STATIC)

qt_extract_metatypes(KF6Notifications)

target_sources(KF6Notifications PRIVATE
  knotification.cpp
  knotificationreplyaction.cpp
  knotificationmanager.cpp
  knotificationpermission.cpp

  knotifyconfig.cpp
  knotificationplugin.cpp
)

if (TARGET Qt6::DBus)
  target_sources(KF6Notifications PRIVATE
    imageconverter.cpp #needed to marshal images for sending over dbus by NotifyByPopup
    notifybypopup.cpp
    notifybyportal.cpp
  )
endif()

if (WITH_SNORETOAST)
    target_sources(KF6Notifications PRIVATE notifybysnore.cpp)
endif ()

if (APPLE)
    target_sources(KF6Notifications PRIVATE notifybymacosnotificationcenter.mm)
endif()

if (TARGET Qt6::DBus)
  set(notifications_xml org.freedesktop.Notifications.xml)
  qt_add_dbus_interface(knotifications_dbus_SRCS ${notifications_xml} notifications_interface)
  target_sources(KF6Notifications PRIVATE ${knotifications_dbus_SRCS})
endif()

configure_file(config-knotifications.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-knotifications.h )

target_link_libraries(KF6Notifications PUBLIC Qt6::Gui)
if (TARGET Qt6::DBus)
   target_link_libraries(KF6Notifications PUBLIC Qt6::DBus)
endif()

if (TARGET SnoreToast::SnoreToastActions)
  target_link_libraries(KF6Notifications PRIVATE Qt6::Network SnoreToast::SnoreToastActions)
endif ()

if(APPLE)
  target_link_libraries(KF6Notifications PRIVATE "-framework Foundation" "-framework AppKit")
endif()

target_include_directories(KF6Notifications PUBLIC
	${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_library(KF6::Notifications ALIAS KF6Notifications)
