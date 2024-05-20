set(CANTATA_TAGS_SRCS main.cpp taghelper.cpp tags.cpp filetyperesolver.cpp ../mpd-interface/song.cpp)

if (WIN32)
    set(CMAKE_BUILD_TYPE "Release")
    ADD_EXECUTABLE(cantata-tags WIN32 ${CANTATA_TAGS_SRCS})
    install(TARGETS cantata-tags DESTINATION ${CMAKE_INSTALL_PREFIX})
elseif (APPLE)
    set(CMAKE_BUILD_TYPE "Release")
    ADD_EXECUTABLE(cantata-tags ${CANTATA_TAGS_SRCS})
    #install(TARGETS cantata-tags DESTINATION ${MACOSX_BUNDLE_APP_DIR})
else ()
    ADD_EXECUTABLE(cantata-tags ${CANTATA_TAGS_SRCS})
    install(TARGETS cantata-tags RUNTIME DESTINATION ${LINUX_LIB_DIR}/cantata)
endif ()

# TODO: Find out where this is used to replace it.
add_definitions(-DCANTATA_TAG_SERVER -DCANTATA_NO_UI_FUNCTIONS)
target_link_libraries(cantata-tags
	Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Core
	Qt${QT_VERSION_MAJOR}::Network TagLib::TagLib)
