#[===[.rst:
FindMTP
-------

Finds the UDev Libraries.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``UDev::UDev``
  The udev library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``UDev_FOUND``
  True if the system has the UDev libraries.
``UDev_VERSION``
  The version of UDev which was found.
``UDev_INCLUDE_DIRS``
  Include directories needed to use UDev.
``UDev_LIBRARIES``
  Libraries needed to link to UDev.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``UDev_LIBRARY``
  The path to the UDev library.
``UDev_INCLUDE_DIR``
  The directory containing ``libudev.h``.
#]===]

# First use PKG-Config as a starting point.
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_UDev QUIET libudev)
endif(PKG_CONFIG_FOUND)

find_path(UDev_INCLUDE_DIR
    NAMES libudev.h
    PATHS ${PC_UDev_INCLUDE_DIRS}
)

find_library(UDev_LIBRARY
    NAMES udev
    PATHS ${PC_UDev_LIBRARY_DIRS}
)

# Set version from PC if applicable.
set(MTP_VERSION ${PC_UDev_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UDev
    FOUND_VAR UDev_FOUND
    REQUIRED_VARS
        UDev_INCLUDE_DIR
        UDev_LIBRARY
    VERSION_VAR UDev_VERSION
)

if(UDev_FOUND)
    set(UDev_LIBRARIES ${UDev_LIBRARY})
    set(UDev_INCLUDE_DIRS ${UDev_INCLUDE_DIR})
    set(UDev_DEFINITIONS ${PC_UDev_CFLAGS_OTHER})
    if(NOT TARGET UDev::UDev)
        add_library(UDev::UDev UNKNOWN IMPORTED)
        set_target_properties(UDev::UDev PROPERTIES
            IMPORTED_LOCATION "${UDev_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_UDev_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${UDev_INCLUDE_DIR}"
        )
    endif ()
endif()

mark_as_advanced(
        UDev_INCLUDE_DIR
        UDev_LIBRARY
)