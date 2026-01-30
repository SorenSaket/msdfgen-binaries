cmake_minimum_required(VERSION 3.15)

project(msdfgen-binaries VERSION 1.13.0 LANGUAGES CXX)

# Options
option(MSDFGEN_USE_SKIA "Build with the Skia library for geometry preprocessing" ON)
option(BUILD_SHARED_LIBS "Generate dynamic library files instead of static" ON)
option(MSDFGEN_BUILD_STANDALONE "Build the msdfgen standalone executable" OFF)

# Force shared libraries
set(BUILD_SHARED_LIBS ON CACHE BOOL "Generate shared libraries" FORCE)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Position independent code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find dependencies via vcpkg
find_package(Freetype REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(PNG REQUIRED)

if(MSDFGEN_USE_SKIA)
    find_package(unofficial-skia CONFIG REQUIRED)
    find_package(Threads REQUIRED)
endif()

# msdfgen source directory
set(MSDFGEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/msdfgen)

# Check if msdfgen submodule exists
if(NOT EXISTS "${MSDFGEN_DIR}/msdfgen.h")
    message(FATAL_ERROR "msdfgen submodule not found. Run: git submodule update --init --recursive")
endif()

# Collect source files
file(GLOB_RECURSE MSDFGEN_CORE_SOURCES "${MSDFGEN_DIR}/core/*.cpp")
file(GLOB_RECURSE MSDFGEN_CORE_HEADERS "${MSDFGEN_DIR}/core/*.h" "${MSDFGEN_DIR}/core/*.hpp")
file(GLOB_RECURSE MSDFGEN_EXT_SOURCES "${MSDFGEN_DIR}/ext/*.cpp")
file(GLOB_RECURSE MSDFGEN_EXT_HEADERS "${MSDFGEN_DIR}/ext/*.h" "${MSDFGEN_DIR}/ext/*.hpp")

# ============================================================================
# msdfgen-core library
# ============================================================================
add_library(msdfgen-core SHARED
    ${MSDFGEN_CORE_SOURCES}
    ${MSDFGEN_CORE_HEADERS}
    "${MSDFGEN_DIR}/msdfgen.h"
)

target_include_directories(msdfgen-core PUBLIC
    $<BUILD_INTERFACE:${MSDFGEN_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(msdfgen-core PUBLIC MSDFGEN_USE_CPP11)

# Handle MSDFGEN_PUBLIC for DLL export/import on Windows
if(WIN32)
    target_compile_definitions(msdfgen-core PRIVATE "MSDFGEN_PUBLIC=__declspec(dllexport)")
    target_compile_definitions(msdfgen-core INTERFACE "MSDFGEN_PUBLIC=__declspec(dllimport)")
else()
    target_compile_definitions(msdfgen-core PUBLIC MSDFGEN_PUBLIC=)
endif()

target_compile_features(msdfgen-core PUBLIC cxx_std_11)

# Export symbols
if(WIN32)
    set_target_properties(msdfgen-core PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# ============================================================================
# msdfgen-ext library
# ============================================================================
add_library(msdfgen-ext SHARED
    ${MSDFGEN_EXT_SOURCES}
    ${MSDFGEN_EXT_HEADERS}
    "${MSDFGEN_DIR}/msdfgen-ext.h"
)

target_include_directories(msdfgen-ext PUBLIC
    $<BUILD_INTERFACE:${MSDFGEN_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(msdfgen-ext PUBLIC
    msdfgen-core
    Freetype::Freetype
)

target_link_libraries(msdfgen-ext PRIVATE
    tinyxml2::tinyxml2
    PNG::PNG
)

target_compile_definitions(msdfgen-ext PUBLIC
    MSDFGEN_USE_CPP11
    MSDFGEN_USE_TINYXML2
    MSDFGEN_USE_LIBPNG
)

# Handle MSDFGEN_EXT_PUBLIC for DLL export/import on Windows
if(WIN32)
    target_compile_definitions(msdfgen-ext PRIVATE "MSDFGEN_EXT_PUBLIC=__declspec(dllexport)")
    target_compile_definitions(msdfgen-ext INTERFACE "MSDFGEN_EXT_PUBLIC=__declspec(dllimport)")
else()
    target_compile_definitions(msdfgen-ext PUBLIC MSDFGEN_EXT_PUBLIC=)
endif()

if(MSDFGEN_USE_SKIA)
    target_compile_definitions(msdfgen-ext PUBLIC MSDFGEN_USE_SKIA)
    target_link_libraries(msdfgen-ext PRIVATE
        unofficial::skia::skia
        Threads::Threads
    )
    target_compile_features(msdfgen-ext PUBLIC cxx_std_17)
endif()

if(WIN32)
    set_target_properties(msdfgen-ext PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# ============================================================================
# msdfgen-c C API wrapper library
# ============================================================================
add_library(msdfgen-c SHARED
    "${CMAKE_CURRENT_SOURCE_DIR}/src/msdfgen_c_api.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/msdfgen_c_api.h"
)

target_include_directories(msdfgen-c PRIVATE
    ${MSDFGEN_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(msdfgen-c PRIVATE
    msdfgen-core
    msdfgen-ext
)

target_compile_features(msdfgen-c PRIVATE cxx_std_17)

set_target_properties(msdfgen-c PROPERTIES
    OUTPUT_NAME "msdfgen_c"
)

if(WIN32)
    set_target_properties(msdfgen-c PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    target_compile_definitions(msdfgen-c PRIVATE MSDFGEN_C_API_EXPORTS)
else()
    target_compile_options(msdfgen-c PRIVATE -fvisibility=hidden)
endif()

# ============================================================================
# Output directories
# ============================================================================
set_target_properties(msdfgen-core msdfgen-ext msdfgen-c PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# ============================================================================
# Standalone executable (optional)
# ============================================================================
if(MSDFGEN_BUILD_STANDALONE)
    add_executable(msdfgen "${MSDFGEN_DIR}/main.cpp")
    target_compile_definitions(msdfgen PRIVATE MSDFGEN_STANDALONE)
    target_link_libraries(msdfgen PRIVATE msdfgen-core msdfgen-ext)
    set_target_properties(msdfgen PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

# ============================================================================
# Installation
# ============================================================================
include(GNUInstallDirs)

install(TARGETS msdfgen-core msdfgen-ext msdfgen-c
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/msdfgen_c_api.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
