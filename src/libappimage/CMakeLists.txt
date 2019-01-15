cmake_minimum_required(VERSION 3.2)

add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(desktop_integration)

set(
    libappimage_sources

    libappimage.c
    libappimage.cpp
    $<TARGET_OBJECTS:core>
    $<TARGET_OBJECTS:appimage_desktop_integration>
    $<TARGET_OBJECTS:appimage_utils>
)
add_library(libappimage_static STATIC ${libappimage_sources})
add_library(libappimage SHARED ${libappimage_sources})

foreach(target libappimage libappimage_static)
    configure_libappimage_module(${target})
    target_link_libraries(${target}
        PRIVATE libarchive
        PRIVATE xdg-basedir
        PRIVATE XdgUtils
        PRIVATE libappimage_hashlib
        # not linking publicly to squashfuse as headers are not needed when using libappimage
        # unit tests etc., which use squashfuse directly, must link to it explicitly
        PRIVATE libsquashfuse
        PRIVATE xz
        PRIVATE Boost
        PUBLIC libappimage_shared
        PUBLIC pthread
        PRIVATE libgio
        PRIVATE libzlib
        PRIVATE libcairo
        PUBLIC dl
        PRIVATE -static-libgcc
        PRIVATE -static-libstdc++
        )

    target_include_directories(${target} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
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
