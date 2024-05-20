#[===[.rst:
FindLibCDIOParanoia
---------

Finds the libcdio_paranoia Libraries.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``LibCDIOParanoia::CDParanoia``
  The CDParanoia on LibCDIO library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``LibCDIOParanoia_FOUND``
  True if the system has the libcdio_paranoia libraries.
``LibCDIOParanoia_VERSION``
  The version of LibCDIOParanoia which was found.
``LibCDIOParanoia_INCLUDE_DIRS``
  Include directories needed to use LibCDIOParanoia.
``LibCDIOParanoia_LIBRARIES``
  Libraries needed to link to LibCDIOParanoia.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``LibCDIOParanoia_LIBRARY``
  The path to the LibCDIOParanoia library.
``LibCDIOParanoia_INCLUDE_DIR``
  The directory containing ``paranoia.h``.
#]===]

# First use PKG-Config as a starting point.
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_LibCDIOParanoia QUIET libcdio_paranoia)
endif(PKG_CONFIG_FOUND)

find_path(LibCDIOParanoia_INCLUDE_DIR
        NAMES paranoia.h
        PATHS ${PC_LibCDIOParanoia_INCLUDE_DIRS}
        PATH_SUFFIXES cdio cdio/paranoia
)

find_library(LibCDIOParanoia_LIBRARY
        NAMES cdio_paranoia
        PATHS ${PC_LibCDIOParanoia_LIBRARY_DIRS}
)

# Set version from PC if applicable.
set(LibCDIOParanoia_VERSION ${PC_LibCDIOParanoia_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibCDIOParanoia
        FOUND_VAR LibCDIOParanoia_FOUND
        REQUIRED_VARS
        LibCDIOParanoia_INCLUDE_DIR
        LibCDIOParanoia_LIBRARY
        VERSION_VAR LibCDIOParanoia_VERSION
)

if(LibCDIOParanoia_FOUND)
    set(LibCDIOParanoia_LIBRARIES ${LibCDIOParanoia_LIBRARY})
    set(LibCDIOParanoia_INCLUDE_DIRS ${LibCDIOParanoia_INCLUDE_DIR})
    set(LibCDIOParanoia_DEFINITIONS ${PC_LibCDIOParanoia_CFLAGS_OTHER})
    if(NOT TARGET LibCDIOParanoia::CDParanoia)
        add_library(LibCDIOParanoia::CDParanoia UNKNOWN IMPORTED)
        set_target_properties(LibCDIOParanoia::CDParanoia PROPERTIES
                IMPORTED_LOCATION "${LibCDIOParanoia_LIBRARY}"
                INTERFACE_COMPILE_OPTIONS "${PC_LibCDIOParanoia_CFLAGS_OTHER}"
                INTERFACE_INCLUDE_DIRECTORIES "${LibCDIOParanoia_INCLUDE_DIR}"
        )
    endif ()
    find_package(LibCDIOCDDA REQUIRED)
    find_package(LibCDIO REQUIRED)
	target_link_libraries(LibCDIOParanoia::CDParanoia INTERFACE LibCDIOParanoia::CDDA LibCDIO::CDIO)
endif()

mark_as_advanced(
        LibCDIOParanoia_INCLUDE_DIR
        LibCDIOParanoia_LIBRARY
)
