cmake_minimum_required(VERSION 3.10)

set(SINGLEAPP-SOURCES
    qtlocalpeer.cpp
    qtlockedfile.cpp
    qtsingleapplication.cpp
    qtsinglecoreapplication.cpp
)

if(WIN32)
    set(SINGLEAPP-SOURCES ${SINGLEAPP-SOURCES} qtlockedfile_win.cpp)
else()
    set(SINGLEAPP-SOURCES ${SINGLEAPP-SOURCES} qtlockedfile_unix.cpp)
endif()

add_library(qtsingleapplication STATIC ${SINGLEAPP-SOURCES})

target_link_libraries(qtsingleapplication PRIVATE ${QT_LIBS} ZLIB::ZLIB)
