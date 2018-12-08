cmake_minimum_required(VERSION 3.2)
include(configure_appimage_module_helper.cmake)

set(libappimage_public_header ${PROJECT_SOURCE_DIR}/include/appimage/appimage.h)

add_subdirectory(core)
add_subdirectory(utils)

# both libraries use the same set of source files
set(libappimage_sources
    ${libappimage_public_header}
    libappimage.c
    appimage_handler.h
    appimage_handler.c
    type1.c
    type2.c
    desktop_integration.h
    desktop_integration.c
    appimage_handler.h
    )

add_library(libappimage SHARED ${libappimage_sources})
add_library(libappimage_static STATIC ${libappimage_sources})
add_library(libappimage_0.2 SHARED $<TARGET_OBJECTS:appimage_core> $<TARGET_OBJECTS:appimage_utils>)

foreach(target libappimage libappimage_static libappimage_0.2)
    configure_appimage_module(${target})
endforeach()


# install libappimage
install(TARGETS libappimage
    EXPORT libappimageTargets
    LIBRARY DESTINATION lib COMPONENT libappimage
    ARCHIVE DESTINATION lib/static COMPONENT libappimage
    PUBLIC_HEADER DESTINATION include/appimage COMPONENT libappimage-dev
    )

# Add all targets to the build-tree export set
export(
    TARGETS libappimage libappimage_shared libappimage_hashlib
    FILE "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/libappimageTargets.cmake"
)
