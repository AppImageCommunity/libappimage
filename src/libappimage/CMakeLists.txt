cmake_minimum_required(VERSION 3.6)

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
if(BUILD_SHARED_LIBS)
    add_library(libappimage SHARED ${libappimage_sources} libappimage_legacy.cpp)

    # For shared libraries, link privately to xdg-basedir
    target_link_libraries(libappimage PRIVATE xdg-basedir)
else()
    add_library(libappimage STATIC ${libappimage_sources})
    # For static libraries, include xdg-basedir object files directly to avoid export issues
    target_sources(libappimage PRIVATE $<TARGET_OBJECTS:xdg-basedir>)
    get_target_property(XDG_BASEDIR_INCLUDES xdg-basedir INTERFACE_INCLUDE_DIRECTORIES)
    if(XDG_BASEDIR_INCLUDES)
        target_include_directories(libappimage PRIVATE ${XDG_BASEDIR_INCLUDES})
    endif()
endif()

configure_file(config.h.in ${PROJECT_BINARY_DIR}/generated-headers/appimage/config.h)

configure_libappimage_module(libappimage)

target_link_libraries(
    libappimage
    PRIVATE libarchive
    PRIVATE XdgUtils::DesktopEntry
    PRIVATE XdgUtils::BaseDir
    PRIVATE libappimage_hashlib
    # not linking publicly to squashfuse as headers are not needed when using libappimage
    # unit tests etc., which use squashfuse directly, must link to it explicitly
    PRIVATE libsquashfuse
    PRIVATE Boost::boost
    PUBLIC libappimage_shared
    PUBLIC pthread
    PRIVATE libgio
    PUBLIC libcairo
    PUBLIC librsvg
    PUBLIC dl
)
message(STATUS "IMAGE_MANIPULATION_BACKEND_LIBS ${IMAGE_MANIPULATION_BACKEND_LIBS}")
if(LIBAPPIMAGE_STANDALONE)
    target_link_libraries(libappimage PRIVATE -static-libgcc -static-libstdc++)
endif()

target_include_directories(
    libappimage
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated-headers>
)

set_property(TARGET libappimage PROPERTY PUBLIC_HEADER ${libappimage_public_header})

set_property(TARGET libappimage PROPERTY VERSION ${libappimage_VERSION})
set_property(TARGET libappimage PROPERTY SOVERSION ${libappimage_SOVERSION})

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

install(
    DIRECTORY ${PROJECT_BINARY_DIR}/generated-headers/appimage/
    DESTINATION include/appimage
    COMPONENT libappimage-dev
)

# Add all targets to the build-tree export set
export(
    TARGETS libappimage libappimage_shared libappimage_hashlib
    FILE "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/libappimageTargets.cmake"
)
