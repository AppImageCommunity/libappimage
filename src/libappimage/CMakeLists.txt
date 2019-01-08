cmake_minimum_required(VERSION 3.2)

add_subdirectory(utils)
add_subdirectory(desktop_integration)

set(libappimage_sources
    ${libappimage_public_header}

    libappimage.cpp
    core/AppImage.cpp
    core/Traversal.h
    core/FileStream.cpp
    core/FilesIterator.cpp
    core/impl/TraversalFallback.cpp
    core/impl/TraversalType1.cpp
    core/impl/TraversalType2.cpp
    core/impl/StreambufFallback.h
    core/impl/StreambufType1.cpp
    core/impl/StreambufType2.cpp

    # legacy C implementation
    legacy/libappimage.c
    legacy/appimage_handler.c
    legacy/type1.c
    legacy/type2.c
    legacy/desktop_integration.c
)

add_library(libappimage_static STATIC ${libappimage_sources} $<TARGET_OBJECTS:appimage_utils>)
add_library(libappimage SHARED ${libappimage_sources} $<TARGET_OBJECTS:appimage_utils>)

foreach(target libappimage libappimage_static)
    configure_libappimage_module(${target})
    target_link_libraries(${target}
        PRIVATE libarchive
        PRIVATE xdg-basedir
        # not linking publicly to squashfuse as headers are not needed when using libappimage
        # unit tests etc., which use squashfuse directly, must link to it explicitly
        PRIVATE libsquashfuse
        PRIVATE xz
        PRIVATE Boost
        PUBLIC libappimage_shared
        PUBLIC pthread
        PUBLIC libglib
        PUBLIC libgobject
        PUBLIC libgio
        PUBLIC libzlib
        PUBLIC libcairo
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
