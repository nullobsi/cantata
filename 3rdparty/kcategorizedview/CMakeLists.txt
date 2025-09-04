cmake_minimum_required(VERSION 3.5...3.31)

set(KCAT_SOURCES
    kcategorizedsortfilterproxymodel.cpp
    kcategorizedview.cpp
    kcategorydrawer.cpp
)

add_definitions(-DKITEMVIEWS_NO_DEPRECATED)

add_library(kcategorizedview STATIC ${KCAT_SOURCES})
target_link_libraries(kcategorizedview PRIVATE Qt6::Widgets)

target_include_directories(
	kcategorizedview PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
