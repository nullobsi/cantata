# - Find CoreFoundation on Mac
#
#  CoreFoundation_LIBRARY - the library to use CoreFoundation
#  CoreFoundation_FOUND - true if CoreFoundation has been found

# Copyright (c) 2009, Harald Fernengel <harry@kdevelop.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindPackageHandleStandardArgs)

find_library(CoreFoundation_LIBRARY CoreFoundation)

find_package_handle_standard_args(CoreFoundation DEFAULT_MSG CoreFoundation_LIBRARY)

if(CoreFoundation_FOUND)
	set(CoreFoundation_LIBRARIES ${CoreFoundation_LIBRARY})
	add_library(CoreFoundation::CoreFoundation UNKNOWN IMPORTED)
	set_target_properties(CoreFoundation::CoreFoundation PROPERTIES
		IMPORTED_LOCATION "${CoreFoundation_LIBRARY}"
	)
endif()
