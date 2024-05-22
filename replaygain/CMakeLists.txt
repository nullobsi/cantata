if(FFMPEG_FOUND OR MPG123_FOUND)
    add_executable(cantata-replaygain)
    target_link_libraries(
        cantata-replaygain
        PRIVATE
            Qt${QT_VERSION_MAJOR}::Core
            Qt${QT_VERSION_MAJOR}::Gui
            ZLIB::ZLIB
    )
    target_sources(
        cantata-replaygain
        PRIVATE
            main.cpp
            replaygain.cpp
            trackscanner.cpp
            jobcontroller.cpp
            ../support/thread.cpp
    )
    target_include_directories(
        cantata-replaygain
        PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
    )

    if(FFMPEG_FOUND)
        target_sources(cantata-replaygain PRIVATE ffmpeginput.cpp)
        target_link_libraries(
            cantata-replaygain
            PRIVATE FFMPEG::avutil FFMPEG::avcodec FFMPEG::avformat
        )
    endif()

    if(MPG123_FOUND)
        target_sources(cantata-replaygain PRIVATE mpg123input.cpp)
        target_link_libraries(cantata-replaygain PRIVATE MPG123::MPG123)
    endif()

    find_package(EBUR128)

    if(EBUR128_FOUND)
        message("-- Using system libebur128")
        target_link_libraries(cantata-replaygain PRIVATE EBUR128::EBUR128)
    else()
        message("-- Using supplied libebur128")
        add_subdirectory(
            ${CMAKE_SOURCE_DIR}/3rdparty/ebur128
            ${CMAKE_BINARY_DIR}/3rdparty/ebur128
        )
        target_link_libraries(cantata-replaygain PRIVATE ebur128)
    endif()

    if(UNIX AND NOT APPLE)
        target_link_libraries(cantata-replaygain PRIVATE pthread)
    endif()

    if(WIN32)
        set_property(TARGET cantata-replaygain PROPERTY WIN32_EXECUTABLE true)
        # Install to root of prefix.
        install(TARGETS cantata-replaygain RUNTIME DESTINATION ".")
    elseif(APPLE)
        install(TARGETS cantata-replaygain RUNTIME DESTINATION ${APP_MACOS_DIR})
    else()
        install(
            TARGETS cantata-replaygain
            RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/${PROJECT_NAME}
        )
    endif()
endif()
