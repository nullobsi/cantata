# - Find IOKit on Mac
#
#  IOKit_LIBRARY - the library to use IOKit
#  IOKit_FOUND - true if IOKit has been found

# Copyright (c) 2009, Harald Fernengel <harry@kdevelop.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
include(FindPackageHandleStandardArgs)

find_package(CoreFoundation REQUIRED)
find_library(IOKit_LIBRARY IOKit)

find_package_handle_standard_args(IOKit DEFAULT_MSG IOKit_LIBRARY)

if(IOKit_FOUND)
	set(IOKit_LIBRARIES ${IOKit_LIBRARY})
	add_library(IOKit::IOKit UNKNOWN IMPORTED)
	set_target_properties(IOKit::IOKit PROPERTIES
		IMPORTED_LOCATION "${IOKit_LIBRARY}"
	)
	target_link_libraries(IOKit::IOKit INTERFACE CoreFoundation::CoreFoundation)
endif()
