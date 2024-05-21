set (ebur128_SRCS ebur128.c)
add_library (ebur128 STATIC ${ebur128_SRCS})
if(MSVC)
    target_compile_definitions(ebur128 PRIVATE _USE_MATH_DEFINES __SSE2_MATH__)
    target_compile_options(ebur128 PRIVATE "/arch:SSE2")
endif()

target_include_directories(ebur128 PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})