set(CANTATA_INSTALL_STREAM_ICONS stream.png)

if(WIN32)
    install(
        FILES ${CANTATA_INSTALL_STREAM_ICONS}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/icons/
    )
elseif(APPLE)
    install(
        FILES ${CANTATA_INSTALL_STREAM_ICONS}
        DESTINATION ${APP_RESOURCES_DIR}/icons/
    )
else()
    install(
        FILES ${CANTATA_INSTALL_STREAM_ICONS}
        DESTINATION ${SHARE_INSTALL_PREFIX}/${CMAKE_PROJECT_NAME}/icons/
    )
endif()
