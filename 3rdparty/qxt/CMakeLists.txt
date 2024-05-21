cmake_minimum_required(VERSION 2.6)

set(QXT-SOURCES
  qxtglobal.cpp
  qxtglobalshortcut.cpp
)

if (NOT WIN32 AND NOT APPLE)
  find_package(X11)
  include_directories(${X11_INCLUDE_DIR})
endif ()

if(WIN32)
  set(QXT-SOURCES ${QXT-SOURCES} qxtglobalshortcut_win.cpp)
elseif(APPLE)
  set(QXT-SOURCES ${QXT-SOURCES} qxtglobalshortcut_mac.cpp)
else()
  set(QXT-SOURCES ${QXT-SOURCES} qxtglobalshortcut_x11.cpp)
endif()

ADD_LIBRARY(qxt STATIC
  ${QXT-SOURCES}
)

target_link_libraries(qxt PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui)
