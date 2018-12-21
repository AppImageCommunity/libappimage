cmake_minimum_required(VERSION 3.2)

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

set(libappimage_0.2_sources
    ${libappimage_public_header}
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
)

add_library(libappimage SHARED ${libappimage_sources})
add_library(libappimage_static STATIC ${libappimage_sources})
add_library(libappimage_0.2 SHARED ${libappimage_0.2_sources} $<TARGET_OBJECTS:appimage_utils>)

foreach(target libappimage libappimage_static libappimage_0.2)
    configure_libappimage_module(${target})
    target_link_libraries(${target}
        PRIVATE libarchive
        PRIVATE xdg-basedir
        # not linking publicly to squashfuse as headers are not needed when using libappimage
        # unit tests etc., which use squashfuse directly, must link to it explicitly
        PRIVATE libsquashfuse
        PRIVATE xz
        PUBLIC libappimage_shared
        PUBLIC pthread
        PUBLIC libglib
        PUBLIC libgobject
        PUBLIC libgio
        PUBLIC libzlib
        PUBLIC libcairo
    )
endforeach()
target_link_libraries(libappimage_0.2 PRIVATE Boost)
target_include_directories(libappimage_0.2 PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

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
