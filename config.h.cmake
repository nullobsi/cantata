#ifndef _CONFIG_H
#define _CONFIG_H

#include <QCoreApplication>
#include "support/utils.h"

#if QT_VERSION >= 0x050600
#define DEVICE_PIXEL_RATIO devicePixelRatioF
#else
#define DEVICE_PIXEL_RATIO devicePixelRatio
#endif

#define CANTATA_MAKE_VERSION(a, b, c) (((a) << 16) | ((b) << 8) | (c))
#define PACKAGE_NAME  "@PROJECT_NAME@"
#define ORGANIZATION_NAME "@ORGANIZATION_NAME@"
#define PACKAGE_VERSION CANTATA_MAKE_VERSION(@PROJECT_VERSION_MAJOR@, @PROJECT_VERSION_MINOR@, @PROJECT_VERSION_PATCH@)
#define PACKAGE_STRING  PACKAGE_NAME" @PROJECT_VERSION@"
#define PACKAGE_VERSION_STRING "@PROJECT_VERSION@"
// TODO: Fix these hardcoded paths.
#define INSTALL_PREFIX "@CMAKE_INSTALL_PREFIX@"
#define SHARE_INSTALL_PREFIX "@SHARE_INSTALL_PREFIX@"
#define ICON_INSTALL_PREFIX "@ICON_INSTALL_PREFIX@"

#cmakedefine ENABLE_DEVICES_SUPPORT 1
#cmakedefine ENABLE_REMOTE_DEVICES 1
#cmakedefine TagLib_FOUND 1
#cmakedefine MTP_FOUND 1
#cmakedefine ENABLE_HTTP_STREAM_PLAYBACK 1
#cmakedefine FFMPEG_FOUND 1
#cmakedefine MPG123_FOUND 1
#cmakedefine CDDB_FOUND 1
#cmakedefine MusicBrainz5_FOUND 1
#cmakedefine ENABLE_REPLAYGAIN_SUPPORT 1
#cmakedefine ENABLE_PROXY_CONFIG 1
#cmakedefine CDPARANOIA_HAS_CACHEMODEL_SIZE 1
#cmakedefine LibCDIOParanoia_FOUND 1
#cmakedefine QT_QTDBUS_FOUND 1
#cmakedefine ENABLE_HTTP_SERVER 1
#cmakedefine IOKIT_FOUND 1
#cmakedefine ENABLE_SIMPLE_MPD_SUPPORT 1
#cmakedefine Avahi_FOUND 1
#cmakedefine ENABLE_CATEGORIZED_VIEW 1
#cmakedefine ENABLE_SCROBBLING 1

#define PROJECT_REV_ID "@PROJECT_REV_ID@"
#define PROJECT_ID "@PROJECT_ID@"

#define CANTATA_SYS_ICONS_DIR   Utils::systemDir(QLatin1String("icons"))
#define CANTATA_SYS_MPD_DIR     Utils::systemDir(QLatin1String("mpd"))
#define CANTATA_SYS_TRANS_DIR   Utils::systemDir(QLatin1String("translations"))
#define CANTATA_SYS_SCRIPTS_DIR Utils::systemDir(QLatin1String("scripts"))

#define LINUX_LIB_DIR "@LINUX_LIB_DIR@"

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define CANTATA_ENDL Qt::endl
#define CANTATA_SKIP_EMPTY Qt::SkipEmptyParts
#define CANTATA_GET_LABEL_PIXMAP(L) L->pixmap(Qt::ReturnByValue)
#else
#define CANTATA_ENDL endl
#define CANTATA_SKIP_EMPTY QString::SkipEmptyParts
#define CANTATA_GET_LABEL_PIXMAP(L) (*(L->pixmap()))
#endif

#endif
