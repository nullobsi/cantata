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
    # Newer development versions of musicbrainz split the library into
    # C++ and C versions.
    pkg_check_modules(PC_MusicBrainz5cc QUIET libmusicbrainz5cc)
    pkg_check_modules(PC_MusicBrainz5 QUIET libmusicbrainz5)
endif(PKG_CONFIG_FOUND)

find_path(MusicBrainz5_INCLUDE_DIR
    NAMES Disc.h
    PATHS ${PC_MusicBrainz5_INCLUDE_DIRS} ${PC_MusicBrainz5cc_INCLUDE_DIRS}
    PATH_SUFFIXES "musicbrainz5"
)

find_library(MusicBrainz5cc_LIBRARY
    NAMES musicbrainz5cc
    PATHS ${PC_MusicBrainz5_LIBRARY_DIRS}
)

find_library(MusicBrainz5_LIBRARY
    NAMES musicbrainz5
    PATHS ${PC_MusicBrainz5cc_LIBRARY_DIRS}
)

# Set version from PC if applicable.
if (MusicBrainz5cc_LIBRARY)
    set(MusicBrainz5_VERSION ${PC_MusicBrainz5cc_VERSION})
    # Copy library from musicbrain5cc
    set(MusicBrainz5_LIBRARY ${MusicBrainz5cc_LIBRARY})
elseif (MusicBrainz5_LIBRARY)
    set(MusicBrainz5_VERSION ${PC_MusicBrainz5_VERSION})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MusicBrainz5
    FOUND_VAR MusicBrainz5_FOUND
    REQUIRED_VARS
        MusicBrainz5_INCLUDE_DIR
        MusicBrainz5_LIBRARY
    VERSION_VAR MusicBrainz5_VERSION
)

if(MusicBrainz5_FOUND)
    set(MusicBrainz5_INCLUDE_DIRS ${MusicBrainz5_INCLUDE_DIR})
    if(MusicBrainz5cc_LIBRARY)
        set(MusicBrainz5_DEFINITIONS ${PC_MusicBrainz5cc_CFLAGS_OTHER})
        set(MusicBrainz5_LIBRARIES ${MusicBrainz5cc_LIBRARY})
    else()
        set(MusicBrainz5_DEFINITIONS ${PC_MusicBrainz5_CFLAGS_OTHER})
        set(MusicBrainz5_LIBRARIES ${MusicBrainz5_LIBRARY})
    endif()
    if(NOT TARGET MusicBrainz5::MusicBrainz)
        add_library(MusicBrainz5::MusicBrainz UNKNOWN IMPORTED)
        if(MusicBrainz5cc_LIBRARY)
            set_target_properties(MusicBrainz5::MusicBrainz PROPERTIES
                IMPORTED_LOCATION "${MusicBrainz5cc_LIBRARY}"
                INTERFACE_COMPILE_OPTIONS "${PC_MusicBrainz5cc_CFLAGS_OTHER}"
                INTERFACE_INCLUDE_DIRECTORIES "${MusicBrainz5_INCLUDE_DIR}"
            )
        else()
            set_target_properties(MusicBrainz5::MusicBrainz PROPERTIES
                IMPORTED_LOCATION "${MusicBrainz5_LIBRARY}"
                INTERFACE_COMPILE_OPTIONS "${PC_MusicBrainz5_CFLAGS_OTHER}"
                INTERFACE_INCLUDE_DIRECTORIES "${MusicBrainz5_INCLUDE_DIR}"
            )
        endif()
    endif ()
endif()

mark_as_advanced(
        MusicBrainz5_INCLUDE_DIR
        MusicBrainz5_LIBRARY
)
