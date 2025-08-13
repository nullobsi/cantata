cmake_minimum_required(VERSION 3.5...3.29)

project(
    Cantata
    VERSION "3.3.1"
    DESCRIPTION "Graphical MPD Client with Qt."
    HOMEPAGE_URL "https://github.com/nullobsi/cantata"
    LANGUAGES CXX C
)

#  NOTE: If PROJECT_URL, or PROJECT_REV_URL, are changed, then cantata-dynamic, README, and
#        dbus/mpd.cantata.xml will need renaming/updating.
set(PROJECT_ID "Cantata.cantata.unix.dog")
set(PROJECT_REV_ID "dog.unix.cantata.Cantata")
set(EXECUTABLE_NAME "cantata")

message(STATUS "Configuring ${PROJECT_NAME} v${PROJECT_VERSION}")

# Set module path.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Set CPack variables. Version info & name were given by the project
# command.
set(CPACK_PACKAGE_VENDOR ${PROJECT_NAME})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README")
set(CPACK_PACKAGE_EXECUTABLES "${EXECUTABLE_NAME};${PROJECT_NAME}")

# TODO: CPACK_PACKAGE_DESCRIPTION & etc for installers

set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")

set(CPACK_SOURCE_IGNORE_FILES
    "/.svn/;/.git/;.gitignore;.github/;.project;CMakeLists.txt.user;README.md;/screenshots/"
)

# Now set some variables used in config.h.

set(ORGANIZATION_NAME ${PROJECT_NAME})

# Install dirs
include(GNUInstallDirs)

# Packaging variables.
if(WIN32)
    set(ICON_INSTALL_PREFIX icons)

    # CPack.
    set(CPACK_GENERATOR WIX ZIP)

    # WIX .msi generator
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/windows/cantata.ico")
    set(CPACK_WIX_PROPERTY_APPHELPLINK "${PROJECT_HOMEPAGE_URL}")
    set(CPACK_WIX_PROPERTY_APPURLINFOABOUT "${PROJECT_HOMEPAGE_URL}")
    set(CPACK_WIX_ROOT_FEATURE_DESCRIPTION "${PROJECT_DESCRIPTION}")

    set(CPACK_WIX_UPGRADE_GUID "5a353104-c90b-4467-a48c-2204681a5383")

    # WiX doesn't like files without extensions.
    configure_file(
        "${CMAKE_SOURCE_DIR}/LICENSE"
        "${CMAKE_BINARY_DIR}/LICENSE.txt"
        COPYONLY
    )
    configure_file(
        "${CMAKE_SOURCE_DIR}/README"
        "${CMAKE_BINARY_DIR}/README.txt"
        COPYONLY
    )

    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/LICENSE.txt")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_BINARY_DIR}/README.txt")
elseif(APPLE)
    # CPack.
    set(CPACK_GENERATOR "DragNDrop")

    # Set default properties for target.
    set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
    set(MACOSX_BUNDLE_VERSION ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright © 2024 Cantata Contributors.")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER ${PROJECT_ID})
    set(MACOSX_BUNDLE_EXECUTABLE cantata)
    set(MACOSX_BUNDLE_ICON_FILE cantata.icns)
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})

    # Bundle installation shenanigans.
    set(APPS "\${CMAKE_INSTALL_PREFIX}/${MACOSX_BUNDLE_BUNDLE_NAME}.app")
    set(DIRS ${CMAKE_BINARY_DIR})

    # Extra variables used to install plugins and resources
    set(APP_BUNDLE_DIR ${PROJECT_NAME}.app)
    set(APP_CONTENTS_DIR ${APP_BUNDLE_DIR}/Contents)
    set(APP_RESOURCES_DIR ${APP_CONTENTS_DIR}/Resources)
    set(APP_MACOS_DIR ${APP_CONTENTS_DIR}/MacOS)
    set(ICON_INSTALL_PREFIX ${APP_RESOURCES_DIR}/icons/cantata)

    set(CPACK_PACKAGE_EXECUTABLES "${APP_BUNDLE_DIR};Cantata")
elseif(NOT HAIKU)
    set(ICON_INSTALL_PREFIX ${CMAKE_INSTALL_DATADIR}/icons/hicolor)

    set(CPACK_GENERATOR TGZ)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "kOpenBSD.*|OpenBSD.*")
    set(OPENBSD TRUE)
endif()

set(QT_MIN_VERSION 5.15)

include(CPack)
include(MacroLogFeature)

# Optional feature flags.
option(
    ENABLE_TAGLIB
    "Enable TagLib library (required for tag editing, replaygain calculation, device support, etc)"
    ON
)
option(
    ENABLE_HTTP_STREAM_PLAYBACK
    "Enable playback of MPD HTTP streams (LibVLC or QtMultimedia)"
    ON
)
option(
    ENABLE_FFMPEG
    "Enable ffmpeg/libav libraries (required for replaygain calculation)"
    ON
)
option(
    ENABLE_MPG123
    "Enable mpg123 libraries (required for replaygain calculation)"
    ON
)
option(ENABLE_PROXY_CONFIG "Enable proxy config in settings dialog" ON)
option(
    ENABLE_HTTP_SERVER
    "Enable internal HTTP server to play non-MPD files"
    ON
)
option(
    ENABLE_LIBVLC
    "Use libVLC for MPD HTTP stream playback (if ENABLE_HTTP_STREAM_PLAYBACK=ON)"
    OFF
)
option(ENABLE_SCROBBLING "Enable scrobbling" ON)

if(WIN32 OR APPLE OR HAIKU)
    option(ENABLE_DEVICES_SUPPORT "Enable suport for external devices" OFF)
	option(BUNDLED_KCATEGORIZEDVIEW "Compile vendored KCategorizedView" ON)
	option(BUNDLED_FONTAWESOME "Use bundled QResources for FontAwesome." ON)
else()
    option(ENABLE_DEVICES_SUPPORT "Enable suport for external devices" ON)
	option(BUNDLED_KCATEGORIZEDVIEW "Compile vendored KCategorizedView" OFF)
	option(BUNDLED_FONTAWESOME "Use bundled QResources for FontAwesome." OFF)
    option(
        ENABLE_REMOTE_DEVICES
        "Enable support for remote (sshfs, samba) devices"
        ON
    )
    option(
        ENABLE_SIMPLE_MPD_SUPPORT
        "Enable support for simple (Cantata controlled) MPD"
        ON
    )
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        option(ENABLE_UDISKS2 "Build UDisks2 backend, and NOT UDisks" ON)
    else()
        option(ENABLE_UDISKS2 "Build UDisks2 backend, and NOT UDisks" OFF)
    endif()
endif()
option(
    ENABLE_CDPARANOIA
    "Enable CDParanoia libraries (this or libCDIO_paranoia required for AudioCD support)"
    ON
)
option(
    ENABLE_CDIOPARANOIA
    "Enable libCDIO_paranoia libraries (this or CDParanoia required for AudioCD support)"
    ON
)
option(
    ENABLE_CDDB
    "Enable CDDB libraries (either this or MusicBrianz required for AudioCD support)"
    ON
)
option(
    ENABLE_MUSICBRAINZ
    "Enable MusicBrainz libraries (either this or CDDB required for AudioCD support)"
    ON
)
option(ENABLE_MTP "Enable MTP library (required to support MTP devices)" ON)
option(ENABLE_AVAHI "Enable automatic mpd server discovery" ${UNIX})

# Build all apps into top-level folder, so that can run dev versions without install
if(NOT WIN32 AND NOT APPLE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTOUIC TRUE)
set(CMAKE_AUTORCC TRUE)

add_executable(cantata)

set_property(TARGET cantata PROPERTY CXX_STANDARD 17)
set_property(TARGET cantata PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET cantata PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_options(
    cantata
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall
        -Wextra
        -Wpedantic>
)
#    -Wno-cast-qual -Wno-float-equal -Wno-gnu-zero-variadic-macro-arguments \

if(ENABLE_AVAHI)
    find_package(Avahi)
    macro_log_feature(Avahi_FOUND "Avahi Support" "Automatic MPD-Server Detection" "https://www.avahi.org/" FALSE "" "")
endif()

if(Avahi_FOUND)
    target_sources(
        cantata
        PRIVATE gui/findmpddialog.cpp gui/avahidiscovery.cpp gui/avahipoll.cpp
    )
    target_link_libraries(cantata PRIVATE Avahi::Client)
endif()

if(NOT WIN32 AND NOT APPLE AND ENABLE_DEVICES_SUPPORT AND ENABLE_UDISKS2)
    set(WITH_SOLID_UDISKS2 ON)
endif()

if(ENABLE_TAGLIB)
    if(NOT TagLib_FOUND)
        find_package(TagLib 2.0 QUIET)
    endif()
    macro_log_feature(TagLib_FOUND "TagLib"     "Tag editor, file organiser, etc."     "http://taglib.github.io/")
endif()

if(TagLib_FOUND)
    if(ENABLE_DEVICES_SUPPORT AND (ENABLE_CDIOPARANOIA OR ENABLE_CDPARANOIA))
        # Issue 1021 - prefer libcdio_paranoia over plain cdparanoia
        if(ENABLE_CDIOPARANOIA)
            find_package(LibCDIOParanoia)
            macro_log_feature(LibCDIOParanoia_FOUND "libcdio_paranoia" "CD ripping." "https://www.gnu.org/software/libcdio/")
        endif()
        if(NOT LibCDIOParanoia_FOUND)
            find_package(CDParanoia)
            macro_log_feature(CDParanoia_FOUND "libcdda_paranoia" "CD ripping." "http://www.xiph.org/paranoia")
        endif()
        if(LibCDIOParanoia_FOUND OR CDParanoia_FOUND)
            if(ENABLE_CDDB)
                find_package(CDDB)
                macro_log_feature(CDDB_FOUND "libcddb" "CD info retrieval via CDDB." "http://libcddb.sourceforge.net")
            endif()
            if(ENABLE_MUSICBRAINZ)
                find_package(MusicBrainz5)
                macro_log_feature(MusicBrainz5_FOUND "libmusicbrainz5" "CD info retrieval via MusicBrainz." "http://musicbrainz.org/doc/libmusicbrainz")
            endif()
        endif()
    endif()

    set(ENABLE_TAGEDITOR_SUPPORT 1)
    set(ENABLE_TRACKORGANIZER_SUPPORT 1)

    if(ENABLE_FFMPEG)
        find_package(FFMPEG COMPONENTS avcodec avutil avformat)
        macro_log_feature(FFMPEG_FOUND "libavcodec/libavutil/libavformat" "ReplayGain calculation." "http://ffmpeg.org")
    endif()
    if(ENABLE_MPG123)
        find_package(MPG123)
        macro_log_feature(MPG123_FOUND "libmpg123" "ReplayGain calculation." "http://www.mpg123.de")
    endif()
    if(ENABLE_MTP AND ENABLE_DEVICES_SUPPORT AND NOT WIN32 AND NOT APPLE)
        find_package(MTP)
        macro_log_feature(MTP_FOUND "libmtp" "MTP Device Support." "http://libmtp.sourceforge.net")
    endif()
else(TagLib_FOUND)
    set(ENABLE_DEVICES_SUPPORT OFF)
endif()

set(QT_COMPONENTS
    Widgets
    Xml
    Network
    Concurrent
    Svg
    Sql
)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS})
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_COMPONENTS})

if(APPLE OR WIN32 OR HAIKU)
    get_target_property(
        QT_QMAKE_EXECUTABLE
        Qt${QT_VERSION_MAJOR}::qmake
        IMPORTED_LOCATION
    )
    execute_process(
        COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_TRANSLATIONS
        OUTPUT_VARIABLE QT_TRANSLATIONS_DIR
    )

    # Find deployment helpers.
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    if(APPLE)
        find_program(
            MACDEPLOYQT_EXECUTABLE
            "macdeployqt${QT_VERSION_MAJOR}"
            HINTS "${QT_BIN_DIR}"
        )
    endif()
    if(WIN32)
        find_program(
            WINDEPLOYQT_EXECUTABLE
            "windeployqt${QT_VERSION_MAJOR}.exe"
            HINTS "${QT_BIN_DIR}"
        )
    endif()
else()
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS DBus)
    set(QT_QTDBUS_FOUND 1) # required for config.h !!!
    list(APPEND QT_COMPONENTS DBus)
endif()

if(ENABLE_HTTP_STREAM_PLAYBACK)
    if(ENABLE_LIBVLC)
        # TODO: VLC as an imported target.
        find_package(LIBVLC REQUIRED)
        macro_log_feature(LIBVLC_FOUND "libVLC" "MPD HTTP stream playback." "http://videolan.org")
        target_compile_definitions(cantata PRIVATE LIBVLC_FOUND)
        target_include_directories(cantata PRIVATE ${LIBVLC_INCLUDE_DIR})
        target_link_libraries(cantata PRIVATE ${LIBVLC_LIBRARY})
    else()
        find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia)
        macro_log_feature(Qt${QT_VERSION_MAJOR}Multimedia_FOUND "QtMultimedia" "MPD HTTP stream playback." "http://qtproject.org")
        list(APPEND QT_COMPONENTS Multimedia)
    endif()
    target_sources(cantata PRIVATE mpd-interface/httpstream.cpp)
endif()

find_package(ZLIB REQUIRED)

if(APPLE)
    target_sources(cantata PRIVATE mac/dockmenu.cpp)
    find_package(IOKit)
    if(IOKIT_FOUND)
        target_link_libraries(cantata PRIVATE ${IOKIT_LIBRARY})
        target_sources(cantata PRIVATE mac/powermanagement.cpp)
    endif()
    find_package(Foundation REQUIRED)
    find_library(MAC_APPKIT_LIBRARY AppKit REQUIRED)
    target_link_libraries(
        cantata
        PRIVATE ${FOUNDATION_LIBRARY} ${MAC_APPKIT_LIBRARY}
    )
    find_library(MAC_MEDIAPLAYER_LIBRARY MediaPlayer)
    if(MAC_MEDIAPLAYER_LIBRARY)
        target_sources(cantata PRIVATE mac/macnowplaying.mm)
        target_link_libraries(cantata PRIVATE ${MAC_MEDIAPLAYER_LIBRARY})
        target_compile_definitions(cantata PRIVATE MAC_MEDIAPLAYER_FOUND)
    endif()
elseif(WIN32 AND QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 COMPONENTS WinExtras REQUIRED)
    list(APPEND QT_COMPONENTS WinExtras)
    target_sources(cantata PRIVATE windows/thumbnailtoolbar.cpp)
endif()

list(
    TRANSFORM QT_COMPONENTS
    PREPEND Qt${QT_VERSION_MAJOR}::
    OUTPUT_VARIABLE QT_LIBS
)
target_link_libraries(cantata PRIVATE ${QT_LIBS})

target_include_directories(
    cantata
    PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
)

target_sources(
    cantata
    PRIVATE
        gui/settings.cpp
        gui/application.cpp
        gui/initialsettingswizard.cpp
        gui/mainwindow.cpp
        gui/preferencesdialog.cpp
        gui/customactionssettings.cpp
        gui/interfacesettings.cpp
        gui/playbacksettings.cpp
        gui/serversettings.cpp
        gui/librarypage.cpp
        gui/customactions.cpp
        gui/folderpage.cpp
        gui/trayitem.cpp
        gui/cachesettings.cpp
        gui/coverdialog.cpp
        gui/searchpage.cpp
        gui/stdactions.cpp
        gui/main.cpp
        gui/covers.cpp
        gui/currentcover.cpp
        gui/mpdbrowsepage.cpp
        gui/localfolderpage.cpp
        gui/multimediakeysinterface.cpp
        gui/apikeys.cpp
        gui/apikeyssettings.cpp
        devices/deviceoptions.cpp
        db/librarydb.cpp
        db/mpdlibrarydb.cpp
        widgets/treeview.cpp
        widgets/listview.cpp
        widgets/itemview.cpp
        widgets/categorizedview.cpp
        widgets/autohidingsplitter.cpp
        widgets/nowplayingwidget.cpp
        widgets/actionlabel.cpp
        widgets/playqueueview.cpp
        widgets/groupedview.cpp
        widgets/actionitemdelegate.cpp
        widgets/textbrowser.cpp
        widgets/volumeslider.cpp
        widgets/menubutton.cpp
        widgets/icons.cpp
        widgets/toolbutton.cpp
        widgets/wizardpage.cpp
        widgets/searchwidget.cpp
        widgets/messageoverlay.cpp
        widgets/basicitemdelegate.cpp
        widgets/sizegrip.cpp
        widgets/spacerwidget.cpp
        widgets/songdialog.cpp
        widgets/stretchheaderview.cpp
        widgets/tableview.cpp
        widgets/thinsplitterhandle.cpp
        widgets/coverwidget.cpp
        widgets/ratingwidget.cpp
        widgets/notelabel.cpp
        widgets/selectorlabel.cpp
        widgets/titlewidget.cpp
        widgets/multipagewidget.cpp
        widgets/singlepagewidget.cpp
        widgets/stackedpagewidget.cpp
        widgets/mirrormenu.cpp
        widgets/genrecombo.cpp
        widgets/volumecontrol.cpp
        context/lyricsettings.cpp
        context/ultimatelyricsprovider.cpp
        context/ultimatelyrics.cpp
        context/lyricsdialog.cpp
        context/contextwidget.cpp
        context/view.cpp
        context/artistview.cpp
        context/albumview.cpp
        context/songview.cpp
        context/contextengine.cpp
        context/wikipediaengine.cpp
        context/wikipediasettings.cpp
        context/othersettings.cpp
        context/contextsettings.cpp
        context/togglelist.cpp
        context/lastfmengine.cpp
        context/metaengine.cpp
        context/onlineview.cpp
        streams/streamspage.cpp
        streams/streamdialog.cpp
        streams/streamfetcher.cpp
        models/streamsproxymodel.cpp
        models/streamsearchmodel.cpp
        models/musiclibraryitemroot.cpp
        models/musiclibraryitemartist.cpp
        models/musiclibraryitemalbum.cpp
        models/musiclibraryproxymodel.cpp
        models/playlistsmodel.cpp
        models/playlistsproxymodel.cpp
        models/playqueuemodel.cpp
        models/proxymodel.cpp
        models/actionmodel.cpp
        models/musiclibraryitem.cpp
        models/browsemodel.cpp
        models/searchmodel.cpp
        models/streamsmodel.cpp
        models/searchproxymodel.cpp
        models/sqllibrarymodel.cpp
        models/mpdlibrarymodel.cpp
        models/mpdsearchmodel.cpp
        models/playqueueproxymodel.cpp
        models/localbrowsemodel.cpp
        mpd-interface/mpdconnection.cpp
        mpd-interface/mpdparseutils.cpp
        mpd-interface/mpdstats.cpp
        mpd-interface/mpdstatus.cpp
        mpd-interface/song.cpp
        mpd-interface/cuefile.cpp
        network/networkaccessmanager.cpp
        network/networkproxyfactory.cpp
        playlists/dynamicplaylists.cpp
        playlists/playlistproxymodel.cpp
        playlists/dynamicplaylistspage.cpp
        playlists/playlistruledialog.cpp
        playlists/playlistrulesdialog.cpp
        playlists/playlistspage.cpp
        playlists/storedplaylistspage.cpp
        playlists/rulesplaylists.cpp
        playlists/smartplaylists.cpp
        playlists/smartplaylistspage.cpp
        online/onlineservicespage.cpp
        online/onlinedbservice.cpp
        online/jamendoservice.cpp
        online/onlinedbwidget.cpp
        online/onlineservice.cpp
        online/jamendosettingsdialog.cpp
        online/magnatuneservice.cpp
        online/magnatunesettingsdialog.cpp
        #online/soundcloudservice.cpp
        online/onlinesearchwidget.cpp
        online/podcastservice.cpp
        online/rssparser.cpp
        online/opmlparser.cpp
        online/podcastsearchdialog.cpp
        online/podcastsettingsdialog.cpp
        online/podcastwidget.cpp
        online/onlinesearchservice.cpp
        db/onlinedb.cpp
        http/httpserver.cpp
        gui/shortcutssettingspage.cpp
        gui/mediakeys.cpp
)

if(ENABLE_SIMPLE_MPD_SUPPORT)
    target_sources(cantata PRIVATE mpd-interface/mpduser.cpp)
endif()
if(ENABLE_SCROBBLING)
    target_sources(
        cantata
        PRIVATE
            scrobbling/scrobbler.cpp
            scrobbling/pausabletimer.cpp
            scrobbling/scrobblingsettings.cpp
            scrobbling/scrobblinglove.cpp
    )
endif()

target_link_libraries(cantata PRIVATE support)

configure_file(
    playlists/cantata-dynamic.cmake
    ${CMAKE_BINARY_DIR}/cantata-dynamic
    @ONLY
)

if(WIN32)
    # Not installed for windows - script uses sym-links...
elseif(APPLE)
    install(
        PROGRAMS ${CMAKE_BINARY_DIR}/cantata-dynamic
        DESTINATION ${APP_RESOURCES_DIR}/scripts/
    )
else()
    install(
        PROGRAMS ${CMAKE_BINARY_DIR}/cantata-dynamic
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/scripts
    )
endif()

if(ENABLE_HTTP_SERVER)
    target_sources(cantata PRIVATE http/httpsocket.cpp)
endif()

if("DBus" IN_LIST QT_COMPONENTS)
    target_sources(
        cantata
        PRIVATE
            dbus/gnomemediakeys.cpp
            dbus/mpris.cpp
            dbus/powermanagement.cpp
    )
    set_source_files_properties(
        dbus/org.gnome.SettingsDaemon.xml
        dbus/org.gnome.SettingsDaemon.MediaKeys.xml
        PROPERTIES NO_NAMESPACE TRUE
    )

    configure_file(
        dbus/cantata.xml.cmake
        ${CMAKE_BINARY_DIR}/${PROJECT_REV_ID}.xml
    )

    qt_add_dbus_adaptor(DBUS_GENERATED_SRCS dbus/org.mpris.MediaPlayer2.Player.xml dbus/mpris.h Mpris)
    qt_add_dbus_adaptor(DBUS_GENERATED_SRCS dbus/org.mpris.MediaPlayer2.root.xml dbus/mpris.h Mpris)
    qt_add_dbus_adaptor(DBUS_GENERATED_SRCS ${CMAKE_BINARY_DIR}/${PROJECT_REV_ID}.xml gui/mainwindow.h MainWindow)
    qt_add_dbus_interfaces(DBUS_GENERATED_SRCS 
        	dbus/org.freedesktop.UPower.xml
        	dbus/org.freedesktop.login1.xml
        	dbus/org.gnome.SettingsDaemon.xml
        	dbus/org.gnome.SettingsDaemon.MediaKeys.xml
        	dbus/org.kde.Solid.PowerManagement.PolicyAgent.xml
        	dbus/org.freedesktop.PowerManagement.Inhibit.xml
    )

    target_sources(cantata PRIVATE ${DBUS_GENERATED_SRCS})
endif()

if(NOT WIN32 AND NOT APPLE)
    target_sources(cantata PRIVATE devices/mountpoints.cpp)
endif()

target_sources(cantata PRIVATE cantata.qrc cantata_media.qrc)

if(TagLib_FOUND)
    target_sources(
        cantata
        PRIVATE
            tags/tageditor.cpp
            tags/trackorganiser.cpp
            devices/filenameschemedialog.cpp
            widgets/tagspinbox.cpp
            tags/taghelperiface.cpp
    )

    # Cantata still links to taglib, even if external tag reader/writer is used, because JamendoService uses taglib for ID3 genres.
    target_link_libraries(cantata PRIVATE TagLib::TagLib)
    if(ENABLE_DEVICES_SUPPORT)
        target_link_libraries(cantata PRIVATE solidlite)
    endif()

    if(FFMPEG_FOUND OR MPG123_FOUND)
        target_sources(
            cantata
            PRIVATE
                replaygain/albumscanner.cpp
                replaygain/rgdialog.cpp
                replaygain/tagreader.cpp
                replaygain/jobcontroller.cpp
        )
        set(ENABLE_REPLAYGAIN_SUPPORT 1)
        add_subdirectory(replaygain)
    endif()
    add_subdirectory(tags)

    if(ENABLE_DEVICES_SUPPORT)
        add_subdirectory(3rdparty/solid-lite)
        if(MTP_FOUND)
            target_sources(cantata PRIVATE devices/mtpdevice.cpp)
            target_link_libraries(cantata PRIVATE MTP::MTP)
        endif()
        if(CDDB_FOUND OR MusicBrainz5_FOUND)
            if(CDDB_FOUND)
                target_sources(cantata PRIVATE devices/cddbinterface.cpp)
                target_link_libraries(cantata PRIVATE CDDB::CDDB)
            endif()
            if(MusicBrainz5_FOUND)
                target_sources(cantata PRIVATE devices/musicbrainz.cpp)
                target_link_libraries(cantata PRIVATE MusicBrainz5::MusicBrainz)
            endif()
            target_sources(
                cantata
                PRIVATE
                    devices/audiocddevice.cpp
                    devices/cddbselectiondialog.cpp
                    devices/cdparanoia.cpp
                    devices/audiocdsettings.cpp
                    devices/extractjob.cpp
                    devices/albumdetailsdialog.cpp
            )
            # If CDDB/MusicBrainz5 found - then CDParanoia must have been!
            if(LibCDIOParanoia_FOUND)
                target_link_libraries(
                    cantata
                    PRIVATE LibCDIOParanoia::CDParanoia
                )
            else()
                target_link_libraries(cantata PRIVATE CDDA::Paranoia)
            endif()
        endif()

        target_sources(
            cantata
            PRIVATE
                devices/devicespage.cpp
                devices/filejob.cpp
                devices/device.cpp
                devices/fsdevice.cpp
                devices/umsdevice.cpp
                devices/splitlabelwidget.cpp
                models/devicesmodel.cpp
                devices/actiondialog.cpp
                devices/devicepropertieswidget.cpp
                devices/devicepropertiesdialog.cpp
                devices/encoders.cpp
                devices/freespaceinfo.cpp
                devices/transcodingjob.cpp
                devices/valueslider.cpp
                devices/syncdialog.cpp
                devices/synccollectionwidget.cpp
                models/musiclibrarymodel.cpp
        )

        if(ENABLE_REMOTE_DEVICES)
            target_sources(
                cantata
                PRIVATE
                    devices/remotefsdevice.cpp
                    devices/remotedevicepropertiesdialog.cpp
                    devices/remotedevicepropertieswidget.cpp
            )
        endif()
    else(ENABLE_DEVICES_SUPPORT)
        target_sources(cantata PRIVATE devices/device.cpp)
    endif()
else(TagLib_FOUND)
    set(ENABLE_DEVICES_SUPPORT OFF)
    set(ENABLE_REMOTE_DEVICES OFF)
endif()

if(ENABLE_PROXY_CONFIG)
    target_sources(cantata PRIVATE network/proxysettings.cpp)
endif()

if(WIN32 OR APPLE OR HAIKU)
    target_sources(cantata PRIVATE gui/singleapplication.cpp)
endif()
if(WIN32)
    target_sources(cantata PRIVATE gui/application_win.cpp)
elseif(APPLE)
    target_sources(cantata PRIVATE gui/application_mac.cpp)
elseif(HAIKU)
    target_sources(cantata PRIVATE gui/application_haiku.cpp)
else()
    target_sources(cantata PRIVATE gui/application_qt.cpp)
endif()
if(WIN32)
    target_compile_definitions(cantata PRIVATE WIN32)
    add_subdirectory(windows)
    target_sources(cantata PRIVATE windows/cantataico.rc)
elseif(APPLE)
    target_compile_definitions(cantata PRIVATE __APPLE__)
endif()

if(WIN32 OR APPLE)
    target_sources(cantata PRIVATE cantata_icon.qrc)
else()
    add_subdirectory(icons)
endif()

if(WIN32)
    set_target_properties(cantata PROPERTIES WIN32_EXECUTABLE TRUE)
    # Install to root of prefix.
    install(TARGETS cantata RUNTIME DESTINATION ".")
elseif(APPLE)
    set_target_properties(
        cantata
        PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_RPATH TRUE
            OUTPUT_NAME ${MACOSX_BUNDLE_BUNDLE_NAME}
            XCODE_ATTRIBUTE_EXECUTABLE_NAME ${EXECUTABLE_NAME}
    )

    add_subdirectory(mac)
    install(
        PROGRAMS
            ${CMAKE_BINARY_DIR}/${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/MacOS/${MACOSX_BUNDLE_BUNDLE_NAME}
        DESTINATION ${APP_MACOS_DIR}
    )
    install(
        FILES
            ${CMAKE_BINARY_DIR}/${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Info.plist
        DESTINATION ${APP_CONTENTS_DIR}
    )
    if(TagLib_FOUND)
        # *NEED* to install cantata-replagain/cantata-tags *before* install_qt5_executable fix'es the bundle
        # If we install from their own CMakeLists.txt then they are installed *after* the fix-up, and so link
        # to the build libs not the packaged ones!
        if(FFMPEG_FOUND OR MPG123_FOUND)
            install(
                PROGRAMS ${CMAKE_BINARY_DIR}/replaygain/cantata-replaygain
                DESTINATION ${APP_MACOS_DIR}
            )
        endif(FFMPEG_FOUND OR MPG123_FOUND)
        install(
            PROGRAMS ${CMAKE_BINARY_DIR}/tags/cantata-tags
            DESTINATION ${APP_MACOS_DIR}
        )
    endif(TagLib_FOUND)
else()
    install(TARGETS cantata)
endif()

if(BUNDLED_KCATEGORIZEDVIEW)
    add_subdirectory(3rdparty/kcategorizedview)
    target_link_libraries(cantata PRIVATE kcategorizedview)
else()
	find_package(KF6ItemViews REQUIRED)
	target_link_libraries(cantata PRIVATE KF6::ItemViews)
endif()

if(WIN32 OR APPLE OR HAIKU)
    add_subdirectory(3rdparty/qtsingleapplication)
    target_link_libraries(cantata PRIVATE qtsingleapplication)
else()
    set(XDG_APPS_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/applications")
endif()

add_subdirectory(translations)
add_subdirectory(support)
add_subdirectory(3rdparty/qtiocompressor)
add_subdirectory(3rdparty/knotifications)
add_subdirectory(streams/icons)
add_subdirectory(online/icons)

target_link_libraries(cantata PRIVATE support-core qtiocompressor
 KF6Notifications)

# enable warnings
add_definitions(-DQT_NO_DEBUG_OUTPUT)

if(UNIX AND NOT APPLE)
    if(NOT HAIKU)
        configure_file(
            cantata-remote.cmake
            ${CMAKE_BINARY_DIR}/cantata-remote
            @ONLY
        )
        configure_file(
            cantata.desktop.cmake
            ${CMAKE_BINARY_DIR}/${PROJECT_REV_ID}.desktop
            @ONLY
        )
        configure_file(
            cantata.metainfo.xml.cmake
            ${CMAKE_BINARY_DIR}/${PROJECT_REV_ID}.metainfo.xml
            @ONLY
        )
        install(
            PROGRAMS ${CMAKE_BINARY_DIR}/cantata-remote
            DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/scripts
        )
        install(
            FILES ${CMAKE_BINARY_DIR}/${PROJECT_REV_ID}.desktop
            DESTINATION ${XDG_APPS_INSTALL_DIR}
        )
        install(
            FILES ${CMAKE_BINARY_DIR}/${PROJECT_REV_ID}.metainfo.xml
            DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo"
        )
    endif()
    target_link_libraries(cantata PRIVATE pthread)
endif()

configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

if(APPLE)
    # Using macdeployqt from the official Qt download works.
    # With homebrew, the libraries are more entangled and it doesn't work.
    install(
        CODE "execute_process(COMMAND ${MACDEPLOYQT_EXECUTABLE} \"${APPS}\")"
    )
    #install(CODE "include(BundleUtilities)
    #	fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")")
    #install(CODE "execute_process(COMMAND ${MACDEPLOYQT_EXECUTABLE} \"${APPS}\")")
    # We still want to codesign it.
    install(CODE "execute_process(COMMAND codesign -f -s \"-\" \"${APPS}\")")
endif()

if(WIN32)
    # Copy libraries.
    install(
        CODE
            "execute_process(COMMAND ${WINDEPLOYQT_EXECUTABLE}
		--no-translations --compiler-runtime --no-system-d3d-compiler
		--no-system-dxc-compiler --no-quick-import
		\"\${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_NAME}.exe\")"
    )
    # Might be dangerous.
    if(MINGW AND NOT NO_MSYS2_BUNDLEDLLS)
        install(
            CODE
                "file(GLOB_RECURSE BUNDLED_DLLS
			\"\${CMAKE_INSTALL_PREFIX}/*.dll\"
			\"\${CMAKE_INSTALL_PREFIX}/cantata.exe\"
			\"\${CMAKE_INSTALL_PREFIX}/cantata-replaygain.exe\"
			\"\${CMAKE_INSTALL_PREFIX}/cantata-tags.exe\")
			foreach(BUNDLE_DLL IN LISTS BUNDLED_DLLS)
				execute_process(COMMAND bash
					\"${CMAKE_SOURCE_DIR}/windows/bundledlls\"
					\"\${BUNDLE_DLL}\" \"\${CMAKE_INSTALL_PREFIX}\")
			endforeach()"
        )
    endif()
endif()

macro_display_feature_log()

if(
    (
        NOT WIN32
        AND NOT APPLE
        AND (
            (NOT ENABLE_SIMPLE_MPD_SUPPORT)
            OR (NOT ENABLE_DEVICES_SUPPORT)
            OR (ENABLE_DEVICES_SUPPORT AND NOT ENABLE_REMOTE_DEVICES)
        )
    )
    OR (NOT ENABLE_HTTP_STREAM_PLAYBACK)
    OR (NOT ENABLE_PROXY_CONFIG)
)
    message(
        "-----------------------------------------------------------------------------"
    )
    message("-- The following STANDARD options have been DISABLED.")
    message(
        "-----------------------------------------------------------------------------"
    )
    if(NOT WIN32 AND NOT APPLE)
        if(NOT ENABLE_SIMPLE_MPD_SUPPORT)
            message(
                "    * Simple/Personal (Cantata controlled) MPD instance support"
            )
        endif()
        if(NOT ENABLE_DEVICES_SUPPORT)
            message(
                "    * Devices Tab - Media device support (UMS, MTP, AudioCD)."
            )
        elseif(NOT ENABLE_REMOTE_DEVICES)
            message(
                "    * Remote Devices - Access sshfs, Samba, and local folders as devices. (EXPERIMENTAL)"
            )
        endif()
    endif()
    if(NOT ENABLE_HTTP_STREAM_PLAYBACK)
        message(
            "    * MPD HTTP Streams - Playback MPD HTTP output stream via libVLC."
        )
    endif()
    if(NOT ENABLE_PROXY_CONFIG)
        message("    * Proxy Configuration")
    endif()

    message("")
endif()
