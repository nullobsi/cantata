cmake_minimum_required(VERSION 2.6)

set(KCAT_SOURCES
    kcategorizedsortfilterproxymodel.cpp
    kcategorizedview.cpp
    kcategorydrawer.cpp
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QTINCLUDES}
)

add_definitions(-DKITEMVIEWS_NO_DEPRECATED)

add_library(kcategorizedview STATIC ${KCAT_SOURCES})
