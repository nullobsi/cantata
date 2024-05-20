#[===[.rst:
FindCDDB
---------

Finds the CDDB Libraries.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``CDDB::CDDB``
  The CDDB library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``CDDB_FOUND``
  True if the system has the CDDB libraries.
``CDDB_VERSION``
  The version of CDDB which was found.
``CDDB_INCLUDE_DIRS``
  Include directories needed to use CDDB.
``CDDB_LIBRARIES``
  Libraries needed to link to CDDB.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``CDDB_LIBRARY``
  The path to the CDDB library.
``CDDB_INCLUDE_DIR``
  The directory containing ``cddb.h``.
#]===]

# First use PKG-Config as a starting point.
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_CDDB QUIET libcddb)
endif(PKG_CONFIG_FOUND)

find_path(CDDB_INCLUDE_DIR
    NAMES cddb.h
    PATHS ${PC_CDDB_INCLUDE_DIRS}
    PATH_SUFFIXES cddb
)

find_library(CDDB_LIBRARY
    NAMES cddb
    PATHS ${PC_CDDB_LIBRARY_DIRS}
)

# Set version from PC if applicable.
set(CDDB_VERSION ${PC_CDDB_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CDDB
    FOUND_VAR CDDB_FOUND
    REQUIRED_VARS
        CDDB_INCLUDE_DIR
        CDDB_LIBRARY
    VERSION_VAR CDDB_VERSION
)

if(CDDB_FOUND)
    set(CDDB_LIBRARIES ${CDDB_LIBRARY})
    set(CDDB_INCLUDE_DIRS ${CDDB_INCLUDE_DIR})
    set(CDDB_DEFINITIONS ${PC_CDDB_CFLAGS_OTHER})
    if(NOT TARGET CDDB::CDDB)
        add_library(CDDB::CDDB UNKNOWN IMPORTED)
        set_target_properties(CDDB::CDDB PROPERTIES
            IMPORTED_LOCATION "${CDDB_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_CDDB_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${CDDB_INCLUDE_DIR}"
        )
    endif ()
endif()

mark_as_advanced(
    CDDB_INCLUDE_DIR
    CDDB_LIBRARY
)