# - Find AppKit on Mac
#
#  AppKit_LIBRARY - the library to use AppKit
#  AppKit_FOUND - true if AppKit has been found

# Copyright (c) 2009, Harald Fernengel <harry@kdevelop.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindPackageHandleStandardArgs)

find_library(AppKit_LIBRARY AppKit)

find_package_handle_standard_args(AppKit DEFAULT_MSG AppKit_LIBRARY)

if(AppKit_FOUND)
	set(AppKit_LIBRARIES ${AppKit_LIBRARY})
	add_library(AppKit::AppKit UNKNOWN IMPORTED)
	set_target_properties(AppKit::AppKit PROPERTIES
		IMPORTED_LOCATION "${AppKit_LIBRARY}"
	)
endif()
