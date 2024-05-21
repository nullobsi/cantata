add_executable(cantata-tags)
target_sources(
    cantata-tags
    PRIVATE
        main.cpp
        taghelper.cpp
        tags.cpp
        filetyperesolver.cpp
        ../mpd-interface/song.cpp
)
target_include_directories(
    cantata-tags
    PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
)

if(WIN32)
    set_property(TARGET cantata-tags PROPERTY WIN32_EXECUTABLE true)
    install(TARGETS cantata-tags)
elseif(APPLE)
    install(TARGETS cantata-tags RUNTIME DESTINATION ${APP_MACOS_DIR})
else()
    install(
        TARGETS cantata-tags
        RUNTIME DESTINATION ${LINUX_LIB_DIR}/${PROJECT_NAME}
    )
endif()

# TODO: Find out where this is used to replace it.
add_definitions(-DCANTATA_TAG_SERVER -DCANTATA_NO_UI_FUNCTIONS)
target_link_libraries(
    cantata-tags
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    TagLib::TagLib
)
