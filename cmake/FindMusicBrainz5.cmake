#[===[.rst:
FindMusicBrainz5
----------------

Finds the MusicBrainz Libraries.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``MusicBrainz5::MusicBrainz``
  The MusicBrainz library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``MusicBrainz5_FOUND``
  True if the system has the MusicBrainz libraries.
``MusicBrainz5_VERSION``
  The version of MusicBrainz which was found.
``MusicBrainz5_INCLUDE_DIRS``
  Include directories needed to use MusicBrainz.
``MusicBrainz5_LIBRARIES``
  Libraries needed to link to MusicBrainz.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``MusicBrainz5_LIBRARY``
  The path to the MusicBrainz library.
``MusicBrainz5_INCLUDE_DIR``
  The directory containing ``Disc.h``.
#]===]

# First use PKG-Config as a starting point.
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_MusicBrainz5 QUIET libmusicbrainz5cc)
endif(PKG_CONFIG_FOUND)

find_path(MusicBrainz5_INCLUDE_DIR
    NAMES Disc.h
    PATHS ${PC_MusicBrainz5_INCLUDE_DIRS}
    PATH_SUFFIXES "musicbrainz5"
)

find_library(MusicBrainz5_LIBRARY
    NAMES musicbrainz5cc
    PATHS ${PC_MusicBrainz5_LIBRARY_DIRS}
)

# Set version from PC if applicable.
set(MusicBrainz5_VERSION ${PC_MusicBrainz5_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MusicBrainz5
    FOUND_VAR MusicBrainz5_FOUND
    REQUIRED_VARS
        MusicBrainz5_INCLUDE_DIR
        MusicBrainz5_LIBRARY
    VERSION_VAR MusicBrainz5_VERSION
)

if(MusicBrainz5_FOUND)
    set(MusicBrainz5_LIBRARIES ${MusicBrainz5_LIBRARY})
    set(MusicBrainz5_INCLUDE_DIRS ${MusicBrainz5_INCLUDE_DIR})
    set(MusicBrainz5_DEFINITIONS ${PC_MusicBrainz5_CFLAGS_OTHER})
    if(NOT TARGET MusicBrainz5::MusicBrainz)
        add_library(MusicBrainz5::MusicBrainz UNKNOWN IMPORTED)
        set_target_properties(MusicBrainz5::MusicBrainz PROPERTIES
            IMPORTED_LOCATION "${MusicBrainz5_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_MusicBrainz5_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${MusicBrainz5_INCLUDE_DIR}"
        )
    endif ()
endif()

mark_as_advanced(
        MusicBrainz5_INCLUDE_DIR
        MusicBrainz5_LIBRARY
)
