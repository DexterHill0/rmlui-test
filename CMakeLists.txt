

cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(rmlui-test VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/Backend.cpp
    src/renderer/Renderer.cpp
    src/renderer/Shader.cpp
    src/platform/Platform.cpp
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

include(cmake/CPM.cmake)

setup_geode_mod(${PROJECT_NAME})

CPMAddPackage(
    NAME zlib
    GITHUB_REPOSITORY madler/zlib
    GIT_TAG v1.3.1
    OPTIONS "ZLIB_USE_STATIC_LIBS ON"
)
add_link_options("-l${zlib_BINARY_DIR}/zlib")
# target_include_directories(zlib 
#                             PUBLIC $<BUILD_INTERFACE:${zlib_BINARY_DIR}>
#                             PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories(zlibstatic 
                            PUBLIC $<BUILD_INTERFACE:${zlib_BINARY_DIR}>
                            PUBLIC $<INSTALL_INTERFACE:include>)
include_directories(${zlib_BINARY_DIR} ${zlib_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} zlibstatic)

CPMAddPackage(
    NAME libpng
    GITHUB_REPOSITORY pnggroup/libpng
    VERSION 1.6.43
    OPTIONS "ZLIB_LIBRARY ${zlib_BINARY_DIR}" "ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR}" "PNG_SHARED OFF"
)
# target_include_directories(png_shared 
#                             PUBLIC $<BUILD_INTERFACE:${libpng_BINARY_DIR}>
#                             PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories(png_static 
                            PUBLIC $<BUILD_INTERFACE:${libpng_BINARY_DIR}>
                            PUBLIC $<INSTALL_INTERFACE:include>)
target_link_libraries(${PROJECT_NAME} png_static)
include_directories(${libpng_BINARY_DIR} ${libpng_SOURCE_DIR})

CPMAddPackage("gh:google/brotli#v1.1.0")
target_include_directories(brotlicommon 
                            PUBLIC $<BUILD_INTERFACE:${brotli_BINARY_DIR}>
                            PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories(brotlidec 
                            PUBLIC $<BUILD_INTERFACE:${brotli_BINARY_DIR}>
                            PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories(brotlienc 
                            PUBLIC $<BUILD_INTERFACE:${brotli_BINARY_DIR}>
                            PUBLIC $<INSTALL_INTERFACE:include>)
include_directories(${brotli_SOURCE_DIR}/c/include)
target_link_libraries(${PROJECT_NAME} brotlicommon brotlidec brotlienc)

CPMAddPackage(
    NAME bzip2
    URL https://gitlab.com/bzip2/bzip2/-/archive/master/bzip2-master.zip
    URL_HASH SHA256=a5e5528a0746212b63c5c318623bd2d637f0a5cbe9f03046155c7c75f819c42c
    OPTIONS "ENABLE_STATIC_LIB 1"
)
target_include_directories(bz2_ObjLib 
                            PUBLIC $<BUILD_INTERFACE:${bzip2_BINARY_DIR}>
                            PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories(bz2_static 
                            PUBLIC $<BUILD_INTERFACE:${bzip2_BINARY_DIR}>
                            PUBLIC $<INSTALL_INTERFACE:include>)
# target_include_directories(bz2
#                             PUBLIC $<BUILD_INTERFACE:${bzip2_BINARY_DIR}>
#                             PUBLIC $<INSTALL_INTERFACE:include>)
include_directories(${bzip2_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} bz2_ObjLib bz2_static)

set(HB_VER "9.0.0")                       
CPMAddPackage(
    NAME harfbuzz
    GITHUB_REPOSITORY harfbuzz/harfbuzz
    GIT_TAG ${HB_VER}
)
target_include_directories(harfbuzz 
                            PUBLIC $<BUILD_INTERFACE:${harfbuzz_BINARY_DIR}>
                            PUBLIC $<INSTALL_INTERFACE:include>)

include_directories(${harfbuzz_SOURCE_DIR}/src)
set(HarfBuzz_INCLUDE_DIR ${harfbuzz_BINARY_DIR})
set(HarfBuzz_LIBRARY ${harfbuzz_SOURCE_DIR})
set(PC_HARFBUZZ_CFLAGS_VERSION ${HB_VER})
target_link_libraries(${PROJECT_NAME} harfbuzz)

CPMAddPackage(
    NAME freetype
    GITHUB_REPOSITORY freetype/freetype
    GIT_TAG VER-2-13-2
    OPTIONS "ZLIB_LIBRARY ${zlib_BINARY_DIR}" "ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR}" "PNG_LIBRARY ${libpng_BINARY_DIR}" "PNG_PNG_INCLUDE_DIR ${libpng_SOURCE_DIR}" "BROTLIDEC_LIBRARIES ${brotli_BINARY_DIR}" "BROTLIDEC_INCLUDE_DIRS ${brotli_SOURCE_DIR}" "BZIP2_LIBRARIES ${bzip2_BINARY_DIR}" "BZIP2_INCLUDE_DIR ${bzip2_SOURCE_DIR}" "PC_HARFBUZZ_LIBDIR ${harfbuzz_BINARY_DIR}" "PC_HARFBUZZ_INCLUDEDIR ${harfbuzz_SOURCE_DIR}"
)
target_include_directories(freetype 
                            PUBLIC $<BUILD_INTERFACE:${freetype_BINARY_DIR}>
                            PUBLIC $<INSTALL_INTERFACE:include>)
include_directories(${freetype_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} freetype)

CPMAddPackage(
    NAME rmlui
    GITHUB_REPOSITORY mikke89/RmlUi
    GIT_TAG 6.0
    OPTIONS "RMLUI_STATIC_LIB" "FREETYPE_LIBRARY ${freetype_BINARY_DIR}" "FREETYPE_INCLUDE_DIRS ${freetype_SOURCE_DIR}"
)

target_link_libraries(${PROJECT_NAME} RmlUi::Core RmlUi::Debugger)
target_include_directories(${PROJECT_NAME} PUBLIC ${rmlui_SOURCE_DIR}/Include ${rmlui_SOURCE_DIR}/Backends)

# set(BACKEND RmlUi_Backend_Win32_GL2)
# target_sources(${PROJECT_NAME} PUBLIC ${rmlui_SOURCE_DIR}/Source/Core/FileInterfaceDefault.cpp)

# set(SHARED_LIBS_DIR ${CMAKE_BINARY_DIR}/shared)
# file(MAKE_DIRECTORY ${SHARED_LIBS_DIR})

# set_target_properties(bz2 PROPERTIES 
#     RUNTIME_OUTPUT_DIRECTORY ${SHARED_LIBS_DIR}
# )

# set_target_properties(png_shared PROPERTIES 
#     RUNTIME_OUTPUT_DIRECTORY ${SHARED_LIBS_DIR}
# )

# set_target_properties(zlib PROPERTIES 
#     RUNTIME_OUTPUT_DIRECTORY ${SHARED_LIBS_DIR}
# )