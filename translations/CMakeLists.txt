file(GLOB TS_FILES cantata_*.ts)

find_package(Qt6LinguistTools ${QT_MIN_VERSION} QUIET)

if (Qt6LinguistTools_FOUND)
    set(catalogname cantata)
    add_custom_target(translations ALL)
    qt6_add_lrelease(translations TS_FILES ${TS_FILES} QM_FILES_OUTPUT_VARIABLE QM_FILES)
    foreach(CUR_QM_FILE ${QM_FILES})
        if (WIN32)
            install(FILES ${CUR_QM_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/translations/)
        elseif (APPLE)
            install(FILES ${CUR_QM_FILE} DESTINATION ${MACOSX_BUNDLE_RESOURCES}/translations/)
        else ()
            install(FILES ${CUR_QM_FILE} DESTINATION ${SHARE_INSTALL_PREFIX}/cantata/translations/)
        endif ()
    endforeach()
else ()
    message("------\n"
            "NOTE: lrelease not found. Translations will *not* be installed\n"
            "------\n")
endif ()

