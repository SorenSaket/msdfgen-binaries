cmake_minimum_required(VERSION 3.15)

# Project configuration
project(msdfgen-binaries VERSION 1.13.0 LANGUAGES CXX)

# Options
option(MSDFGEN_BUILD_STANDALONE "Build the msdfgen standalone executable" OFF)
option(MSDFGEN_USE_SKIA "Build with the Skia library" ON)
option(BUILD_SHARED_LIBS "Generate dynamic library files instead of static" ON)
option(MSDFGEN_USE_VCPKG "Use vcpkg package manager to link project dependencies" ON)
option(MSDFGEN_INSTALL "Generate installation target" OFF)
option(MSDFGEN_USE_CPP11 "Build with C++11 enabled" ON)
option(MSDFGEN_USE_OPENMP "Build with OpenMP support for multithreaded code" OFF)

# Force shared libraries
set(BUILD_SHARED_LIBS ON CACHE BOOL "Generate shared libraries" FORCE)

# vcpkg configuration
if(MSDFGEN_USE_VCPKG)
    if(NOT CMAKE_TOOLCHAIN_FILE)
        if(DEFINED ENV{VCPKG_ROOT})
            set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        else()
            message(FATAL_ERROR "Vcpkg toolchain not configured. Either set VCPKG_ROOT environment variable or pass -DCMAKE_TOOLCHAIN_FILE=VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake to cmake")
        endif()
    endif()
    
    # Set vcpkg features
    set(VCPKG_MANIFEST_NO_DEFAULT_FEATURES ON)
    list(APPEND VCPKG_MANIFEST_FEATURES "extensions")
    if(MSDFGEN_USE_SKIA)
        list(APPEND VCPKG_MANIFEST_FEATURES "geometry-preprocessing")
    endif()
    if(MSDFGEN_BUILD_STANDALONE)
        list(APPEND VCPKG_MANIFEST_FEATURES "standalone")
    endif()
endif()

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add msdfgen as subdirectory
add_subdirectory(msdfgen EXCLUDE_FROM_ALL)

# Custom targets for packaging
if(WIN32)
    set(PLATFORM_SUFFIX "windows-x64")
    set(LIB_EXT "dll")
elseif(APPLE)
    set(PLATFORM_SUFFIX "macos-x64")
    set(LIB_EXT "dylib")
else()
    set(PLATFORM_SUFFIX "linux-x64")
    set(LIB_EXT "so")
endif()

# Create packaging targets
add_custom_target(package-all
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/packages/${PLATFORM_SUFFIX}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:msdfgen-core> ${CMAKE_BINARY_DIR}/packages/${PLATFORM_SUFFIX}/msdfgen-core-${PROJECT_VERSION}-${PLATFORM_SUFFIX}.${LIB_EXT}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:msdfgen-ext> ${CMAKE_BINARY_DIR}/packages/${PLATFORM_SUFFIX}/msdfgen-ext-${PROJECT_VERSION}-${PLATFORM_SUFFIX}.${LIB_EXT}
    VERBATIM
    DEPENDS msdfgen-core msdfgen-ext
)

if(MSDFGEN_BUILD_STANDALONE)
    add_custom_command(TARGET package-all POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:msdfgen> ${CMAKE_BINARY_DIR}/packages/${PLATFORM_SUFFIX}/msdfgen-${PROJECT_VERSION}-${PLATFORM_SUFFIX}
    )
endif()

# Export symbols on Windows
if(BUILD_SHARED_LIBS AND WIN32)
    set_target_properties(msdfgen-core PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set_target_properties(msdfgen-ext PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Output directories
set_target_properties(msdfgen-core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(msdfgen-ext PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(MSDFGEN_BUILD_STANDALONE)
    set_target_properties(msdfgen PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()