cmake_minimum_required(VERSION 3.6)

project(libappimage)

set(V_MAJOR 1)
set(V_MINOR 0)
set(V_PATCH 3)
set(V_SUFFIX "")

set(libappimage_VERSION ${V_MAJOR}.${V_MINOR}.${V_PATCH}${V_SUFFIX})

# more versioning
set(libappimage_SOVERSION ${V_MAJOR}.${V_MINOR})

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimize for size in release builds
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG -Wl,--gc-sections")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

include(cmake/reproducible_builds.cmake)

option(BUILD_SHARED_LIBS "Enable shared library build" On)
option(LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED "Enable desktop integration functions" On)
option(LIBAPPIMAGE_THUMBNAILER_ENABLED "Enable thumbnailer functions" On)
option(LIBAPPIMAGE_STANDALONE "Statically embbed dependencies" Off)
option(LIBAPPIMAGE_SHARED_ONLY "Build and distribute shared and hashlib only (for use with AppImageKit and AppImageUpdate)" Off)
option(ENABLE_COVERAGE "Enable tests code coverate target" Off)

# some dependencies are only checked when testing is enabled
include(CTest)

include(cmake/tools.cmake)
include(cmake/dependencies.cmake)

if(ENABLE_COVERAGE)
    message("Configuring project for code coverage mesurement")
    include(CodeCoverage)
    append_coverage_compiler_flags()
endif()

# used by e.g., Debian packaging infrastructure
include(GNUInstallDirs)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
