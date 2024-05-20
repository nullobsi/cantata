#[===[.rst:
FindLibCDIO
---------

Finds the libcdio Libraries.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``LibCDIO::CDIO``
  The CDIO library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``LibCDIO_FOUND``
  True if the system has the libcdio libraries.
``LibCDIO_VERSION``
  The version of LibCDIO which was found.
``LibCDIO_INCLUDE_DIRS``
  Include directories needed to use LibCDIO.
``LibCDIO_LIBRARIES``
  Libraries needed to link to LibCDIO.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``LibCDIO_LIBRARY``
  The path to the libcdio_cdda library.
``LibCDIO_INCLUDE_DIR``
  The directory containing ``cdio.h``.
#]===]

# First use PKG-Config as a starting point.
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_LibCDIO QUIET libcdio)
endif(PKG_CONFIG_FOUND)

find_path(LibCDIO_INCLUDE_DIR
        NAMES cdio.h
        PATHS ${PC_LibCDIO_INCLUDE_DIRS}
        PATH_SUFFIXES cdio
)

find_library(LibCDIO_LIBRARY
        NAMES cdio_cdda
        PATHS ${PC_LibCDIO_LIBRARY_DIRS}
)

# Set version from PC if applicable.
set(LibCDIO_VERSION ${PC_LibCDIO_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibCDIO
        FOUND_VAR LibCDIO_FOUND
        REQUIRED_VARS
        LibCDIO_INCLUDE_DIR
        LibCDIO_LIBRARY
        VERSION_VAR LibCDIO_VERSION
)

if(LibCDIO_FOUND)
    set(LibCDIO_LIBRARIES ${LibCDIO_LIBRARY} "m")
    set(LibCDIO_INCLUDE_DIRS ${LibCDIO_INCLUDE_DIR})
    set(LibCDIO_DEFINITIONS ${PC_LibCDIO_CFLAGS_OTHER})
    if(NOT TARGET LibCDIO::CDIO)
        add_library(LibCDIO::CDIO UNKNOWN IMPORTED)
		set_target_properties(LibCDIO::CDIO PROPERTIES
                IMPORTED_LOCATION "${LibCDIO_LIBRARY}"
                INTERFACE_COMPILE_OPTIONS "${PC_LibCDIO_CFLAGS_OTHER}"
                INTERFACE_INCLUDE_DIRECTORIES "${LibCDIO_INCLUDE_DIR}"
                INTERFACE_LINK_LIBRARIES "m"
        )
    endif ()
endif()

mark_as_advanced(
        LibCDIO_INCLUDE_DIR
        LibCDIO_LIBRARY
)
