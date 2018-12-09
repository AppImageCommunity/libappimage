cmake_minimum_required(VERSION 3.2)
include(configure_appimage_module_helper.cmake)

set(libappimage_public_header ${PROJECT_SOURCE_DIR}/include/appimage/appimage.h)

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
    core/appimage.cpp
    core/traversal.h
    core/file_istream.cpp
    core/files_iterator.cpp
    core/impl/traversal_fallback.cpp
    core/impl/traversal_type_1.cpp
    core/impl/traversal_type_2.cpp
    core/impl/streambuf_fallback.h
    core/impl/streambuf_type1.cpp
    core/impl/streambuf_type_2.cpp
)

add_library(libappimage SHARED ${libappimage_sources})
add_library(libappimage_static STATIC ${libappimage_sources})
add_library(libappimage_0.2 SHARED ${libappimage_0.2_sources} $<TARGET_OBJECTS:appimage_utils>)

foreach(target libappimage libappimage_static libappimage_0.2)
    configure_appimage_module(${target})
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
