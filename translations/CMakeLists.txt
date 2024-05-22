file(GLOB TS_FILES cantata_*.ts)

find_package(Qt${QT_VERSION_MAJOR}LinguistTools ${QT_MIN_VERSION} QUIET)

if(Qt${QT_VERSION_MAJOR}LinguistTools_FOUND)
    set(catalogname cantata)
    qt_add_lrelease(
            TS_FILES ${TS_FILES}
            LRELEASE_TARGET translations
            QM_FILES_OUTPUT_VARIABLE QM_FILES
    )
    if(WIN32)
        install(FILES ${QM_FILES} DESTINATION translations/)
    elseif(APPLE)
        install(
            FILES ${QM_FILES}
            DESTINATION ${APP_RESOURCES_DIR}/translations/
        )
    else()
        install(
            FILES ${QM_FILES}
            DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/translations/
        )
    endif()
    add_dependencies(cantata translations)
else()
    message(
        "------\n"
        "NOTE: lrelease not found. Translations will *not* be installed\n"
        "------\n"
    )
endif()
