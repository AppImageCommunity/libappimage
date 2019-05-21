cmake_minimum_required(VERSION 3.2)

add_subdirectory(core)
add_subdirectory(utils)

set(
    libappimage_sources
    libappimage.c
    libappimage.cpp
    $<TARGET_OBJECTS:core>
    $<TARGET_OBJECTS:appimage_utils>
)
if(LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED)
    add_subdirectory(desktop_integration)
    list(APPEND libappimage_sources "$<TARGET_OBJECTS:appimage_desktop_integration>")
endif()

add_library(libappimage_static STATIC ${libappimage_sources})
add_library(libappimage SHARED ${libappimage_sources} libappimage_legacy.cpp)

foreach(target libappimage libappimage_static)
    configure_libappimage_module(${target})
    target_link_libraries(
        ${target}
        PRIVATE libarchive
        PRIVATE xdg-basedir
        PRIVATE XdgUtils::DesktopEntry
        PRIVATE XdgUtils::BaseDir
        PRIVATE libappimage_hashlib
        # not linking publicly to squashfuse as headers are not needed when using libappimage
        # unit tests etc., which use squashfuse directly, must link to it explicitly
        PRIVATE libsquashfuse
        PRIVATE xz
        PRIVATE Boost::filesystem
        PUBLIC libappimage_shared
        PUBLIC pthread
        PRIVATE libgio
        PRIVATE libzlib
        PUBLIC libcairo
        PUBLIC librsvg
        PUBLIC dl
    )
    message(STATUS "IMAGE_MANIPULATION_BACKEND_LIBS ${IMAGE_MANIPULATION_BACKEND_LIBS}")
    if(LIBAPPIMAGE_STANDALONE)
        target_link_libraries(${target} PRIVATE -static-libgcc -static-libstdc++)
    endif()

    target_include_directories(${target} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

    set_property(TARGET libappimage PROPERTY PUBLIC_HEADER ${libappimage_public_header})

    set_property(TARGET libappimage PROPERTY VERSION ${libappimage_VERSION})
    set_property(TARGET libappimage PROPERTY SOVERSION ${libappimage_SOVERSION})
endforeach()


# install libappimage
install(
    TARGETS libappimage
    EXPORT libappimageTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libappimage
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libappimage
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/appimage COMPONENT libappimage-dev
)

# install public headers
install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/appimage/
    DESTINATION include/appimage
    COMPONENT libappimage-dev
)

# Add all targets to the build-tree export set
export(
    TARGETS libappimage libappimage_shared libappimage_hashlib
    FILE "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/libappimageTargets.cmake"
)
