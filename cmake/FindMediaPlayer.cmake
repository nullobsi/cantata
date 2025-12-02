# - Find MediaPlayer on Mac
#
#  MediaPlayer_LIBRARY - the library to use MediaPlayer
#  MediaPlayer_FOUND - true if MediaPlayer has been found

# Copyright (c) 2009, Harald Fernengel <harry@kdevelop.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindPackageHandleStandardArgs)

find_library(MediaPlayer_LIBRARY MediaPlayer)

find_package_handle_standard_args(MediaPlayer DEFAULT_MSG MediaPlayer_LIBRARY)

if(MediaPlayer_FOUND)
	set(MediaPlayer_LIBRARIES ${MediaPlayer_LIBRARY})
	add_library(MediaPlayer::MediaPlayer UNKNOWN IMPORTED)
	set_target_properties(MediaPlayer::MediaPlayer PROPERTIES
		IMPORTED_LOCATION "${MediaPlayer_LIBRARY}"
	)
endif()
