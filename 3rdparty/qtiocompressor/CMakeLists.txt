cmake_minimum_required(VERSION 3.5)

add_library(qtiocompressor STATIC qtiocompressor.cpp)
target_link_libraries(
    qtiocompressor
    PUBLIC Qt${QT_VERSION_MAJOR}::Core ZLIB::ZLIB
)
