add_library(KF6Archive STATIC)
add_library(KF6::Archive ALIAS KF6Archive)

set_target_properties(KF6Archive PROPERTIES
    EXPORT_NAME "Archive"
)

target_sources(KF6Archive PRIVATE karchive.cpp
    kar.cpp
    kcompressiondevice.cpp
    kfilterbase.cpp
    kgzipfilter.cpp
    klimitediodevice.cpp
    knonefilter.cpp
    ktar.cpp
    kzip.cpp
    krcc.cpp
)

target_link_libraries(KF6Archive
    PUBLIC
        Qt6::Core
    PRIVATE
        ZLIB::ZLIB
)

target_include_directories(KF6Archive
	PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
