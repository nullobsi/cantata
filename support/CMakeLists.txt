set(SUPPORT_CORE_SRCS utils.cpp thread.cpp)

add_library(support-core STATIC ${SUPPORT_CORE_SRCS})
target_include_directories(
    support-core
    PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
)
target_link_libraries(support-core PRIVATE ${QT_LIBS})

set(SUPPORT_SRCS
    icon.cpp
    fancytabwidget.cpp
    messagewidget.cpp
    buddylabel.cpp
    action.cpp
    actioncollection.cpp
    lineedit.cpp
    configuration.cpp
    gtkstyle.cpp
    spinner.cpp
    messagebox.cpp
    inputdialog.cpp
    thread.cpp
    squeezedtextlabel.cpp
    proxystyle.cpp
    pagewidget.cpp
    combobox.cpp
    configdialog.cpp
        QtAwesome.cpp
    QtAwesomeAnim.cpp
)

if(APPLE)
    set(SUPPORT_SRCS
        ${SUPPORT_SRCS}
        osxstyle.cpp
        flattoolbutton.cpp
        windowmanager.cpp
    )
endif()

set(SUPPORT_SRCS
    ${SUPPORT_SRCS}
    pathrequester.cpp
    kmessagewidget.cpp
    dialog.cpp
    shortcutsmodel.cpp
    shortcutssettingswidget.cpp
    keysequencewidget.cpp
    acceleratormanager.cpp
    urllabel.cpp
)
set(SUPPORT_UIS shortcutssettingswidget.ui)

qt_wrap_ui(SUPPORT_UI_HDRS ${SUPPORT_UIS})
if (BUNDLED_FONTAWESOME)
	qt_add_resources(SUPPORT_RC_SRCS QtAwesomeFree.qrc)
endif()

add_library(
    support
    STATIC
    ${SUPPORT_SRCS}
    ${SUPPORT_UI_HDRS}
    ${SUPPORT_RC_SRCS}
)
target_link_libraries(support PRIVATE ${QT_LIBS})
target_include_directories(
    support
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)
