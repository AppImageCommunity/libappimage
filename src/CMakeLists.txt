# required for pkg-config to create PkgConfig::<prefix> imported library targets
cmake_minimum_required(VERSION 3.6)

add_subdirectory(xdg-basedir)
add_subdirectory(libappimage_hashlib)
add_subdirectory(libappimage_shared)
if(NOT LIBAPPIMAGE_SHARED_ONLY)
    add_subdirectory(libappimage)
endif()

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE libappimage)

include(CMakePackageConfigHelpers)

# Create the AppImageConfig.cmake and AppImageConfigVersion files
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/libappimageConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/libappimageConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libappimage
)
# ... for both
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/libappimageConfigVersion.cmake.in"
    "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/libappimageConfigVersion.cmake"
    @ONLY
)

# Install the AppImageConfig.cmake and AppImageConfigVersion.cmake
install(
    FILES
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/libappimageConfig.cmake"
    "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/libappimageConfigVersion.cmake"
    "${PROJECT_BINARY_DIR}/cmake/scripts.cmake"
    "${PROJECT_BINARY_DIR}/cmake/imported_dependencies.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libappimage
    COMPONENT libappimage-dev
)

# Install the export set for use with the install-tree
install(
    EXPORT libappimageTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libappimage
    COMPONENT libappimage-dev
)

if(NOT LIBAPPIMAGE_SHARED_ONLY)
    # pkg-config
    configure_file("${PROJECT_SOURCE_DIR}/cmake/libappimage.pc.in" "${PROJECT_BINARY_DIR}/libappimage.pc" @ONLY)

    install(FILES "${PROJECT_BINARY_DIR}/libappimage.pc"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        COMPONENT libappimage-dev
    )
endif()
