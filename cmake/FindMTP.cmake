#[===[.rst:
FindMTP
-------

Finds the libmtp Libraries.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``MTP::MTP``
  The libmtp library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``MTP_FOUND``
  True if the system has the libmtp libraries.
``MTP_VERSION``
  The version of libmtp which was found.
``MTP_INCLUDE_DIRS``
  Include directories needed to use libmtp.
``MTP_LIBRARIES``
  Libraries needed to link to libmtp.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``MTP_LIBRARY``
  The path to the libmtp library.
``MTP_INCLUDE_DIR``
  The directory containing ``libmtp.h``.
#]===]

# First use PKG-Config as a starting point.
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_MTP QUIET libmtp)
endif(PKG_CONFIG_FOUND)

find_path(MTP_INCLUDE_DIR
    NAMES libmtp.h
    PATHS ${PC_MTP_INCLUDE_DIRS}
)

find_library(MTP_LIBRARY
    NAMES mtp
    PATHS ${PC_MTP_LIBRARY_DIRS}
)

# Set version from PC if applicable.
set(MTP_VERSION ${PC_MTP_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MTP
    FOUND_VAR MTP_FOUND
    REQUIRED_VARS
        MTP_INCLUDE_DIR
        MTP_LIBRARY
    VERSION_VAR MTP_VERSION
)

if(MTP_FOUND)
  set(MTP_LIBRARIES ${MTP_LIBRARY})
  set(MTP_INCLUDE_DIRS ${MTP_INCLUDE_DIR})
  set(MTP_DEFINITIONS ${PC_MTP_CFLAGS_OTHER})
  if(NOT TARGET MTP::MTP)
      add_library(MTP::MTP UNKNOWN IMPORTED)
      set_target_properties(MTP::MTP PROPERTIES
          IMPORTED_LOCATION "${MTP_LIBRARY}"
          INTERFACE_COMPILE_OPTIONS "${PC_MTP_CFLAGS_OTHER}"
          INTERFACE_INCLUDE_DIRECTORIES "${MTP_INCLUDE_DIR}"
      )
  endif ()
endif()

mark_as_advanced(
    MTP_INCLUDE_DIR
    MTP_LIBRARY
)