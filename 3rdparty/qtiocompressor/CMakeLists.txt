cmake_minimum_required(VERSION 3.5)

ADD_LIBRARY(qtiocompressor STATIC qtiocompressor.cpp)
target_link_libraries(qtiocompressor PRIVATE ${QT_LIBS} )