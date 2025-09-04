cmake_minimum_required(VERSION 3.16)

find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
    URL "https://www.zlib.net"
    DESCRIPTION "Support for gzip compressed files and data streams"
    TYPE REQUIRED
    PURPOSE "Support for gzip compressed files and data streams"
)

add_subdirectory(src)
