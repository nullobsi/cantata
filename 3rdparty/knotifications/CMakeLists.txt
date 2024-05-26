cmake_minimum_required(VERSION 3.16)

# ECM setup
include(FeatureSummary)

include(GenerateExportHeader)

set(REQUIRED_QT_VERSION 6.5.0)
set(CMAKE_AUTORCC TRUE)
set(CMAKE_AUTOMOC TRUE)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Gui)


if (NOT WIN32 AND NOT APPLE)
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
endif()

find_package(Qt6 ${REQUIRED_QT_VERSION} QUIET OPTIONAL_COMPONENTS Qml)

if (WIN32)
    find_package(LibSnoreToast)
    find_package(Qt6Network REQUIRED)
    
    include(CMakeDependentOption)
    cmake_dependent_option(WITH_SNORETOAST "for the Windows Toast Notifications" ON
                           "Qt6Network_FOUND;LibSnoreToast_FOUND" OFF
    )    
endif()

if (NOT APPLE AND NOT WIN32 OR (WIN32 AND NOT WITH_SNORETOAST))
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
endif()

set(HAVE_DBUS FALSE)
if (TARGET Qt6::DBus)
    set(HAVE_DBUS TRUE)
endif()

remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

add_subdirectory(src)
