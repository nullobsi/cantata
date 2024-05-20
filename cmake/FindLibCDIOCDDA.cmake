#[===[.rst:
FindLibCDIOCDDA
---------

Finds the libcdio_CDDA Libraries.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``LibCDIOParanoia::CDDA``
  The CDParanoia CDDA library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``LibCDIOCDDA_FOUND``
  True if the system has the libcdio_CDDA libraries.
``LibCDIOCDDA_VERSION``
  The version of LibCDIOCDDA which was found.
``LibCDIOCDDA_INCLUDE_DIRS``
  Include directories needed to use LibCDIOCDDA.
``LibCDIOCDDA_LIBRARIES``
  Libraries needed to link to LibCDIOCDDA.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``LibCDIOCDDA_LIBRARY``
  The path to the libcdio_cdda library.
``LibCDIOCDDA_INCLUDE_DIR``
  The directory containing ``cdda.h``.
#]===]

# First use PKG-Config as a starting point.
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_LibCDIOCDDA QUIET libcdio_cdda)
endif(PKG_CONFIG_FOUND)

find_path(LibCDIOCDDA_INCLUDE_DIR
        NAMES cdda.h
        PATHS ${PC_LibCDIOCDDA_INCLUDE_DIRS}
        PATH_SUFFIXES cdio cdio/paranoia
)

find_library(LibCDIOCDDA_LIBRARY
        NAMES cdio_cdda
        PATHS ${PC_LibCDIOCDDA_LIBRARY_DIRS}
)

# Set version from PC if applicable.
set(LibCDIOCDDA_VERSION ${PC_LibCDIOCDDA_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibCDIOCDDA
        FOUND_VAR LibCDIOCDDA_FOUND
        REQUIRED_VARS
        LibCDIOCDDA_INCLUDE_DIR
        LibCDIOCDDA_LIBRARY
        VERSION_VAR LibCDIOCDDA_VERSION
)

if(LibCDIOCDDA_FOUND)
    set(LibCDIOCDDA_LIBRARIES ${LibCDIOCDDA_LIBRARY})
    set(LibCDIOCDDA_INCLUDE_DIRS ${LibCDIOCDDA_INCLUDE_DIR})
    set(LibCDIOCDDA_DEFINITIONS ${PC_LibCDIOCDDA_CFLAGS_OTHER})
    if(NOT TARGET LibCDIOParanoia::CDDA)
        add_library(LibCDIOParanoia::CDDA UNKNOWN IMPORTED)
        set_target_properties(LibCDIOParanoia::CDDA PROPERTIES
                IMPORTED_LOCATION "${LibCDIOCDDA_LIBRARY}"
                INTERFACE_COMPILE_OPTIONS "${PC_LibCDIOCDDA_CFLAGS_OTHER}"
                INTERFACE_INCLUDE_DIRECTORIES "${LibCDIOCDDA_INCLUDE_DIR}"
        )
    endif ()
    find_package(LibCDIO REQUIRED)
	target_link_libraries(LibCDIOParanoia::CDDA INTERFACE LibCDIO::CDIO)
endif()

mark_as_advanced(
        LibCDIOCDDA_INCLUDE_DIR
        LibCDIOCDDA_LIBRARY
)
