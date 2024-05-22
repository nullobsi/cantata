if(NOT HAIKU AND NOT WIN32 AND NOT APPLE)
    install(
        FILES cantata.svg
		DESTINATION ${ICON_INSTALL_PREFIX}/scalable/apps
		RENAME ${PROJECT_REV_ID}.svg
    )
    install(
        FILES cantata-symbolic.svg
		DESTINATION ${ICON_INSTALL_PREFIX}/symbolic/apps
		RENAME ${PROJECT_REV_ID}-symbolic.svg
    )
    foreach(
        SIZE
        16
        22
        24
        32
        48
        64
        128
        256
        512
    )
        install(
            FILES cantata${SIZE}.png
			DESTINATION ${ICON_INSTALL_PREFIX}/${SIZE}x${SIZE}/apps
			RENAME ${PROJECT_REV_ID}.png
        )
    endforeach()

    #if (EXISTS /etc/lsb-release)
    #    file(READ "/etc/lsb-release" LSB_RELEASE_CONTENTS)
    #    string(REGEX MATCH "DISTRIB_ID=Ubuntu" IS_UBUNTU ${LSB_RELEASE_CONTENTS})
    #	if (IS_UBUNTU)
    #        set(INSTALL_UBUNTU_ICONS_DEFAULT ON)
    #    else(IS_UBUNTU)
    #        set(INSTALL_UBUNTU_ICONS_DEFAULT OFF)
    #    endif(IS_UBUNTU)
    #else(EXISTS /etc/lsb-release)
    set(INSTALL_UBUNTU_ICONS_DEFAULT OFF)
    #endif(EXISTS /etc/lsb-release)
    option(
        INSTALL_UBUNTU_ICONS
        "Install the Ubuntu Yaru icons"
        ${INSTALL_UBUNTU_ICONS_DEFAULT}
    )

    if(INSTALL_UBUNTU_ICONS)
        add_subdirectory(yaru)
        update_iconcache(Yaru)
    endif()
endif()
