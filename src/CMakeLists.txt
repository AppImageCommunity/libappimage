# required for pkg-config to create PkgConfig::<prefix> imported library targets
cmake_minimum_required(VERSION 3.6)

find_package(PkgConfig)

pkg_check_modules(glib glib-2.0>=2.40 IMPORTED_TARGET)
pkg_check_modules(gobject gobject-2.0>=2.40 IMPORTED_TARGET)
pkg_check_modules(gio gio-2.0>=2.40 IMPORTED_TARGET)
pkg_check_modules(zlib zlib IMPORTED_TARGET)
pkg_check_modules(cairo cairo IMPORTED_TARGET)

add_subdirectory(xdg-basedir)
add_subdirectory(libappimage_hashlib)
add_subdirectory(libappimage_shared)
add_subdirectory(libappimage)

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
install(FILES
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/libappimageConfig.cmake"
    "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/libappimageConfigVersion.cmake"
    "${PROJECT_SOURCE_DIR}/cmake/scripts.cmake"
    "${PROJECT_SOURCE_DIR}/cmake/imported_dependencies.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libappimage
    COMPONENT libappimage-dev
)

# Install the export set for use with the install-tree
install(EXPORT libappimageTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libappimage
    COMPONENT libappimage-dev
)
