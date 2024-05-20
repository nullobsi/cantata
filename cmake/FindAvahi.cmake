#[===[.rst:
FindAvahi
---------

Finds the Avahi Client Libraries.

Input Variables
^^^^^^^^^^^^^^^

The following variables can be set to influence where to search.

``Avahi_COMMON_LIB_DIR``
  The directory containing the Avahi Common library.
``Avahi_CLIENT_LIB_DIR``
  The directory containing the Avahi Client library.
``Avahi_COMMON_INCLUDE_DIR``
  The directory containing the avahi common headers.
``Avahi_CLIENT_INCLUDE_DIR``
  The directory containing the avahi client headers.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Avahi::Common``
  The Avahi Common library.

``Avahi::Client``
  The Avahi Client library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Avahi_FOUND``
  True if the system has the Avahi libraries.
``Avahi_VERSION``
  The version of Avahi which was found.
``Avahi_INCLUDE_DIRS``
  Include directories needed to use Avahi.
``Avahi_LIBRARIES``
  Libraries needed to link to Avahi.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Avahi_COMMON_LIBRARY``
  The path to the Avahi Common library.
``Avahi_CLIENT_LIBRARY``
  The path to the Avahi Client library.
``Avahi_COMMON_INCLUDE_DIR``
  The directory containing the avahi common headers.
``Avahi_CLIENT_INCLUDE_DIR``
  The directory containing the avahi client headers.
#]===]

# First use PKG-Config as a starting point.
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_Avahi QUIET avahi-client)
endif()

# Search for the common library.
find_library(Avahi_COMMON_LIBRARY
    NAMES avahi-common
    PATHS
        ${PC_Avahi_LIBRARY_DIRS}
        ${Avahi_COMMON_LIB_DIR}
)

# Search for the client library.
find_library(Avahi_CLIENT_LIBRARY
    NAMES avahi-client
    PATHS
        ${PC_Avahi_LIBRARY_DIRS}
        ${Avahi_CLIENT_LIB_DIR}
)

# Search for the common include directory.
find_path(Avahi_COMMON_INCLUDE_DIR
    NAMES watch.h
    PATH_SUFFIXES avahi-common
    PATHS
        ${PC_Avahi_INCLUDE_DIRS}
        ${Avahi_COMMON_INCLUDE_DIR}
)

# Search for the client include directory.
find_path(Avahi_CLIENT_INCLUDE_DIR
    NAMES client.h
    PATH_SUFFIXES avahi-client
    PATHS
        ${PC_Avahi_INCLUDE_DIRS}
        ${Avahi_CLIENT_INCLUDE_DIR}
)

# Set version from PC if applicable.
set(Avahi_VERSION ${PC_Avahi_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Avahi
    FOUND_VAR Avahi_FOUND
    REQUIRED_VARS
        Avahi_COMMON_LIBRARY
        Avahi_CLIENT_LIBRARY
        Avahi_COMMON_INCLUDE_DIR
        Avahi_CLIENT_INCLUDE_DIR
    VERSION_VAR Avahi_VERSION
)

if(Avahi_FOUND)
    set(Avahi_LIBRARIES ${Avahi_COMMON_LIBRARY} ${Avahi_CLIENT_LIBRARY})
    set(Avahi_INCLUDE_DIRS ${Avahi_COMMON_INCLUDE_DIR} ${Avahi_CLIENT_INCLUDE_DIR})
    set(Avahi_DEFINITIONS ${PC_Avahi_CFLAGS_OTHER})
    if(NOT TARGET Avahi::Client)
        add_library(Avahi::Client UNKNOWN IMPORTED)
        set_target_properties(Avahi::Client PROPERTIES
            IMPORTED_LOCATION "${Avahi_CLIENT_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_Avahi_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${Avahi_CLIENT_INCLUDE_DIR}"
        )
    endif ()
    if(NOT TARGET Avahi::Common)
        add_library(Avahi::Common UNKNOWN IMPORTED)
        set_target_properties(Avahi::Common PROPERTIES
            IMPORTED_LOCATION "${Avahi_COMMON_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_Avahi_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${Avahi_COMMON_INCLUDE_DIR}"
        )
    endif ()
    target_link_libraries(Avahi::Client INTERFACE Avahi::Common)
endif()

mark_as_advanced(
    Avahi_CLIENT_LIBRARY
    Avahi_CLIENT_INCLUDE_DIR
    Avahi_COMMON_LIBRARY
    Avahi_COMMON_INCLUDE_DIR
)

