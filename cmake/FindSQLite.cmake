#[===[.rst:
FindSQLite
----------

Finds the SQLite Libraries.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``SQLite::SQLite``
  The SQLite library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``SQLite_FOUND``
  True if the system has the SQLite libraries.
``SQLite_VERSION``
  The version of SQLite which was found.
``SQLite_INCLUDE_DIRS``
  Include directories needed to use SQLite.
``SQLite_LIBRARIES``
  Libraries needed to link to SQLite.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``SQLite_LIBRARY``
  The path to the libmtp library.
``SQLite_INCLUDE_DIR``
  The directory containing ``libmtp.h``.
#]===]

# First use PKG-Config as a starting point.
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_SQLite QUIET sqlite3)
endif(PKG_CONFIG_FOUND)

find_path(SQLite_INCLUDE_DIR
    NAMES sqlite3.h
    PATHS ${PC_SQLite_INCLUDE_DIRS}
)

find_library(SQLite_LIBRARY
    NAMES sqlite3
    PATHS ${PC_SQLite_LIBRARY_DIRS}
)

# Set version from PC if applicable.
set(SQLite_VERSION ${PC_SQLite_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SQLite
    FOUND_VAR SQLite_FOUND
    REQUIRED_VARS
        SQLite_INCLUDE_DIR
        SQLite_LIBRARY
    VERSION_VAR SQLite_VERSION
)

if(SQLite_FOUND)
    set(SQLite_LIBRARIES ${SQLite_LIBRARY})
    set(SQLite_INCLUDE_DIRS ${SQLite_INCLUDE_DIR})
    set(SQLite_DEFINITIONS ${PC_SQLite_CFLAGS_OTHER})
    if(NOT TARGET SQLite::SQLite)
        add_library(SQLite::SQLite UNKNOWN IMPORTED)
        set_target_properties(SQLite::SQLite PROPERTIES
            IMPORTED_LOCATION "${SQLite_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_SQLite_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${SQLite_INCLUDE_DIR}"
        )
    endif ()
endif()

mark_as_advanced(
    SQLite_INCLUDE_DIR
    SQLite_LIBRARY
)
