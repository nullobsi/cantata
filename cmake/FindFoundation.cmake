# - Find Foundation on Mac
#
#  Foundation_LIBRARY - the library to use Foundation
#  Foundation_FOUND - true if Foundation has been found

# Copyright (c) 2009, Harald Fernengel <harry@kdevelop.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindPackageHandleStandardArgs)

find_library(Foundation_LIBRARY Foundation)

find_package_handle_standard_args(Foundation DEFAULT_MSG Foundation_LIBRARY)

if(Foundation_FOUND)
	set(Foundation_LIBRARIES ${Foundation_LIBRARY})
	add_library(Foundation::Foundation UNKNOWN IMPORTED)
	set_target_properties(Foundation::Foundation PROPERTIES
		IMPORTED_LOCATION "${Foundation_LIBRARY}"
	)
endif()
