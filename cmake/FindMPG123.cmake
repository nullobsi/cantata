#[===[.rst:
FindMPG123
----------

Finds the MPG123 Libraries.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``MPG123::MPG123``
  The MPG123 library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``MPG123_FOUND``
  True if the system has the MPG123 libraries.
``MPG123_VERSION``
  The version of MPG123 which was found.
``MPG123_INCLUDE_DIRS``
  Include directories needed to use MPG123.
``MPG123_LIBRARIES``
  Libraries needed to link to MPG123.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``MPG123_LIBRARY``
  The path to the mpg123 library.
``MPG123_INCLUDE_DIR``
  The directory containing ``mpg123.h``.
#]===]

# First use PKG-Config as a starting point.
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_MPG123 QUIET libmpg123)
endif(PKG_CONFIG_FOUND)

find_path(MPG123_INCLUDE_DIR
    NAMES mpg123.h
    PATHS ${PC_MPG123_INCLUDE_DIRS}
)

find_library(MPG123_LIBRARY
    NAMES mpg123 mpg123-0
    PATHS ${PC_MPG123_LIBRARY_DIRS}
)

# Set version from PC if applicable.
set(MPG123_VERSION ${PC_MPG123_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MPG123
    FOUND_VAR MPG123_FOUND
    REQUIRED_VARS
        MPG123_INCLUDE_DIR
        MPG123_LIBRARY
    VERSION_VAR MPG123_VERSION
)

if(MPG123_FOUND)
    set(MPG123_LIBRARIES ${MPG123_LIBRARY})
    set(MPG123_INCLUDE_DIRS ${MPG123_INCLUDE_DIR})
    set(MPG123_DEFINITIONS ${PC_MPG123_CFLAGS_OTHER})
    if(NOT TARGET MPG123::MPG123)
        add_library(MPG123::MPG123 UNKNOWN IMPORTED)
        set_target_properties(MPG123::MPG123 PROPERTIES
            IMPORTED_LOCATION "${MPG123_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_MPG123_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${MPG123_INCLUDE_DIR}"
        )
    endif ()
endif()

mark_as_advanced(
    MPG123_INCLUDE_DIR
    MPG123_LIBRARY
)