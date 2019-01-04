cmake_minimum_required(VERSION 3.2)

set(libappimage_public_header ${PROJECT_SOURCE_DIR}/include/appimage/appimage.h)

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
)

add_library(libappimage SHARED ${libappimage_sources} appimage_handler.h)
add_library(libappimage_static STATIC ${libappimage_sources})

# set common options
foreach(target libappimage libappimage_static)
    # targets are called lib* already, so CMake shouldn't add another lib prefix to the actual files
    set_target_properties(${target} PROPERTIES PREFIX "")

    target_compile_definitions(${target}
        # Support Large Files
        PRIVATE -D_FILE_OFFSET_BITS=64
        PRIVATE -D_LARGEFILE_SOURCE

        PRIVATE -DGIT_COMMIT="${GIT_COMMIT}"
        PRIVATE -DENABLE_BINRELOC
    )

    target_include_directories(${target}
        PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
        INTERFACE $<INSTALL_INTERFACE:include/>
    )

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

    set_property(TARGET libappimage PROPERTY PUBLIC_HEADER ${libappimage_public_header})

    set_property(TARGET libappimage PROPERTY VERSION ${V_MAJOR}.${V_MINOR}.${V_PATCH})
    set_property(TARGET libappimage PROPERTY SOVERSION ${V_MAJOR})
endforeach()

# install libappimage
install(TARGETS libappimage
    EXPORT libappimageTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libappimage
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libappimage
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/appimage COMPONENT libappimage-dev
)

# Add all targets to the build-tree export set
export(
    TARGETS libappimage libappimage_shared libappimage_hashlib
    FILE "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/libappimageTargets.cmake"
)
