#[===[.rst:
FindCDParanoia
---------

Finds the CDDA Paranoia Libraries.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``CDDA::Paranoia``
  The CDDA Paranoia library.
``CDDA:Interface``
  The CDDA interface library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``CDParanoia_FOUND``
  True if the system has the CDDA Paranoia libraries.
``CDParanoia_VERSION``
  The version of CDDA Paranoia which was found.
``CDParanoia_INCLUDE_DIRS``
  Include directories needed to use CDDA Paranoia.
``CDParanoia_LIBRARIES``
  Libraries needed to link to CDDA Paranoia.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``CDParanoia_PARANOIA_LIBRARY``
  The path to the CDDA Paranoia library.
``CDParanoia_INTERFACE_LIBRARY``
  The path to the CDDA interface library.
``CDParanoia_PARANOIA_INCLUDE_DIR``
  The directory containing ``cdda_paranoia.h``.
``CDParanoia_INTERFACE_INCLUDE_DIR``
  The directory containing ``cdda_interface.h``.
#]===]

# First use PKG-Config as a starting point.
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_CDParanoia QUIET cdparanoia)
endif(PKG_CONFIG_FOUND)

find_path(CDParanoia_PARANOIA_INCLUDE_DIR
        NAMES cdda_paranoia.h
        PATHS ${PC_CDParanoia_INCLUDE_DIRS}
        PATH_SUFFIXES cdda
)
find_path(CDParanoia_INTERFACE_INCLUDE_DIR
        NAMES cdda_interface.h
        PATHS ${PC_CDParanoia_INCLUDE_DIRS}
        PATH_SUFFIXES cdda
)

find_library(CDParanoia_PARANOIA_LIBRARY
        NAMES cdda_paranoia
        PATHS ${PC_CDParanoia_LIBRARY_DIRS}
)
find_library(CDParanoia_INTERFACE_LIBRARY
        NAMES cdda_interface
        PATHS ${PC_CDParanoia_LIBRARY_DIRS}
)

# Set version from PC if applicable.
set(CDParanoia_VERSION ${PC_CDParanoia_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CDParanoia
        FOUND_VAR CDParanoia_FOUND
        REQUIRED_VARS
            CDParanoia_PARANOIA_INCLUDE_DIR
            CDParanoia_INTERFACE_INCLUDE_DIR
            CDParanoia_PARANOIA_LIBRARY
            CDParanoia_INTERFACE_LIBRARY
        VERSION_VAR CDParanoia_VERSION
)

if(CDParanoia_FOUND)
  set(CDParanoia_LIBRARIES ${CDParanoia_PARANOIA_LIBRARY} ${CDParanoia_INTERFACE_LIBRARY} "m")
  set(CDParanoia_INCLUDE_DIRS ${CDParanoia_PARANOIA_INCLUDE_DIR} ${CDParanoia_INTERFACE_INCLUDE_DIR})
  set(CDParanoia_DEFINITIONS ${PC_CDParanoia_CFLAGS_OTHER})
  if(NOT TARGET CDDA::Paranoia)
    add_library(CDDA::Paranoia UNKNOWN IMPORTED)
    set_target_properties(CDDA::Paranoia PROPERTIES
            IMPORTED_LOCATION "${CDParanoia_PARANOIA_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_CDParanoia_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${CDParanoia_PARANOIA_INCLUDE_DIR}"
    )
  endif ()
  if(NOT TARGET CDDA::Interface)
    set_target_properties(CDDA::Interface PROPERTIES
            IMPORTED_LOCATION "${CDParanoia_INTERFACE_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_CDParanoia_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${CDParanoia_INTERFACE_INCLUDE_DIR}"
    )
    target_link_libraries(CDDA::Interface INTERFACE m)
  endif()
  target_link_libraries(CDDA::Paranoia INTERFACE CDDA::Interface)

  set(CMAKE_REQUIRED_INCLUDES ${CDParanoia_INCLUDE_DIRS})
  set(CMAKE_REQUIRED_LIBRARIES ${CDParanoia_LIBRARIES})
  check_c_source_compiles("#include <cdda_interface.h>
                               #include <cdda_paranoia.h>
                              int main() { paranoia_cachemodel_size(0, 0); return 0; }"
          CDParanoia_HAS_CACHEMODEL_SIZE)
endif()

mark_as_advanced(
        CDParanoia_PARANOIA_INCLUDE_DIR
        CDParanoia_INTERFACE_INCLUDE_DIR
        CDParanoia_PARANOIA_LIBRARY
        CDParanoia_INTERFACE_LIBRARY
)