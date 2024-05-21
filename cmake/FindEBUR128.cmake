#[===[.rst:
FindEBUR128
---------

Finds the EBUR128 Libraries.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``EBUR128::EBUR128``
  The EBUR128 library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``EBUR128_FOUND``
  True if the system has the EBUR128 libraries.
``EBUR128_VERSION``
  The version of EBUR128 which was found.
``EBUR128_INCLUDE_DIRS``
  Include directories needed to use EBUR128.
``EBUR128_LIBRARIES``
  Libraries needed to link to EBUR128.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``EBUR128_LIBRARY``
  The path to the EBUR128 library.
``EBUR128_INCLUDE_DIR``
  The directory containing ``ebur128.h``.
#]===]

# First use PKG-Config as a starting point.
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_EBUR128 QUIET libebur128)
endif(PKG_CONFIG_FOUND)

find_path(EBUR128_INCLUDE_DIR
        NAMES ebur128.h
        PATHS ${PC_EBUR128_INCLUDE_DIRS}
)

find_library(EBUR128_LIBRARY
        NAMES ebur128
        PATHS ${PC_EBUR128_LIBRARY_DIRS}
)

# Set version from PC if applicable.
set(EBUR128_VERSION ${PC_EBUR128_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EBUR128
        FOUND_VAR EBUR128_FOUND
        REQUIRED_VARS
        EBUR128_INCLUDE_DIR
        EBUR128_LIBRARY
        VERSION_VAR EBUR128_VERSION
)

if(EBUR128_FOUND)
  set(EBUR128_LIBRARIES ${EBUR128_LIBRARY})
  set(EBUR128_INCLUDE_DIRS ${EBUR128_INCLUDE_DIR})
  set(EBUR128_DEFINITIONS ${PC_EBUR128_CFLAGS_OTHER})
  if(NOT TARGET EBUR128::EBUR128)
    add_library(EBUR128::EBUR128 UNKNOWN IMPORTED)
    set_target_properties(EBUR128::EBUR128 PROPERTIES
            IMPORTED_LOCATION "${EBUR128_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_EBUR128_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${EBUR128_INCLUDE_DIR}"
    )
  endif ()
endif()

mark_as_advanced(
        EBUR128_INCLUDE_DIR
        EBUR128_LIBRARY
)