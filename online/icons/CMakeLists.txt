set(CANTATA_INSTALL_ONLINE_ICONS podcasts.png soundcloud.png)

if(WIN32)
    install(
        FILES ${CANTATA_INSTALL_ONLINE_ICONS}
        DESTINATION icons/
    )
elseif(APPLE)
    install(
        FILES ${CANTATA_INSTALL_ONLINE_ICONS}
        DESTINATION ${APP_RESOURCES_DIR}/icons/
    )
else()
    install(
        FILES ${CANTATA_INSTALL_ONLINE_ICONS}
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/icons/
    )
endif()
