# >= 3.2 required for ExternalProject_Add_StepDependencies
cmake_minimum_required(VERSION 3.2)

include(${CMAKE_CURRENT_LIST_DIR}/scripts.cmake)

# we use the template both when building from source and in the exported configs
# to make the template work, we have to copy the scripts file to the (future) CMAKE_CURRENT_LIST_DIR (i.e., the parent
# directory of configure_file's target)
file(
    COPY "${CMAKE_CURRENT_LIST_DIR}/scripts.cmake"
    DESTINATION "${PROJECT_BINARY_DIR}/cmake/"
)
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/imported_dependencies.cmake.in"
    "${PROJECT_BINARY_DIR}/cmake/imported_dependencies.cmake"
    @ONLY
)
include("${PROJECT_BINARY_DIR}/cmake/imported_dependencies.cmake")

if(USE_CCACHE)
    message(STATUS "Using CCache to build AppImageKit dependencies")
    # TODO: find way to use find_program with all possible paths
    # (might differ from distro to distro)
    # these work on Debian and Ubuntu:
    set(CC "/usr/lib/ccache/gcc")
    set(CXX "/usr/lib/ccache/g++")
else()
    set(CC "${CMAKE_C_COMPILER}")
    set(CXX "${CMAKE_CXX_COMPILER}")
endif()

set(CFLAGS ${DEPENDENCIES_CFLAGS})
set(CPPFLAGS ${DEPENDENCIES_CPPFLAGS})
set(LDFLAGS ${DEPENDENCIES_LDFLAGS})


if (NOT LIBAPPIMAGE_SHARED_ONLY)
    import_pkgconfig_target(TARGET_NAME xz PKGCONFIG_TARGET liblzma)


    # as distros don't provide suitable squashfuse and squashfs-tools, those dependencies are bundled in, can, and should
    # be used from this repository for AppImageKit
    # for distro packaging, it can be linked to an existing package just fine
    set(USE_SYSTEM_SQUASHFUSE OFF CACHE BOOL "Use system libsquashfuse instead of building our own")

    if(NOT USE_SYSTEM_SQUASHFUSE)
        message(STATUS "Downloading and building squashfuse")

        # Check if fuse is installed to provide early error reports
        import_pkgconfig_target(TARGET_NAME libfuse PKGCONFIG_TARGET fuse)

        # TODO: implement out-of-source builds for squashfuse, as for the other dependencies
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/src/patches/patch-squashfuse.sh.in
            ${CMAKE_CURRENT_BINARY_DIR}/patch-squashfuse.sh
            @ONLY
        )

        ExternalProject_Add(
            squashfuse-EXTERNAL
            GIT_REPOSITORY https://github.com/vasi/squashfuse/
            GIT_TAG 1f98030
            UPDATE_COMMAND ""  # make sure CMake won't try to fetch updates unnecessarily and hence rebuild the dependency every time
            PATCH_COMMAND bash -xe ${CMAKE_CURRENT_BINARY_DIR}/patch-squashfuse.sh
            CONFIGURE_COMMAND ${LIBTOOLIZE} --force
            COMMAND env ACLOCAL_FLAGS="-I /usr/share/aclocal" aclocal
            COMMAND ${AUTOHEADER}
            COMMAND ${AUTOMAKE} --force-missing --add-missing
            COMMAND ${AUTORECONF} -fi || true
            COMMAND ${SED} -i "/PKG_CHECK_MODULES.*/,/,:./d" configure  # https://github.com/vasi/squashfuse/issues/12
            COMMAND ${SED} -i "s/typedef off_t sqfs_off_t/typedef int64_t sqfs_off_t/g" common.h  # off_t's size might differ, see https://stackoverflow.com/a/9073762
            COMMAND CC=${CC} CXX=${CXX} CFLAGS=${CFLAGS} LDFLAGS=${LDFLAGS} <SOURCE_DIR>/configure --disable-demo --disable-high-level --without-lzo --without-lz4 --prefix=<INSTALL_DIR> --libdir=<INSTALL_DIR>/lib --with-xz=${xz_PREFIX} ${EXTRA_CONFIGURE_FLAGS}
            COMMAND ${SED} -i "s|XZ_LIBS = -llzma |XZ_LIBS = -Bstatic ${xz_LIBRARIES}/|g" Makefile
            BUILD_COMMAND ${MAKE}
            BUILD_IN_SOURCE ON
            INSTALL_COMMAND ${MAKE} install
            UPDATE_DISCONNECTED On
        )

        import_external_project(
            TARGET_NAME libsquashfuse
            EXT_PROJECT_NAME squashfuse-EXTERNAL
            LIBRARIES "<SOURCE_DIR>/.libs/libsquashfuse.a;<SOURCE_DIR>/.libs/libsquashfuse_ll.a;<SOURCE_DIR>/.libs/libfuseprivate.a"
            INCLUDE_DIRS "<SOURCE_DIR>"
        )
    else()
        message(STATUS "Using system squashfuse")

        import_pkgconfig_target(TARGET_NAME libsquashfuse PKGCONFIG_TARGET squashfuse)
    endif()


    import_find_pkg_target(libarchive LibArchive LibArchive)


    #### build dependency configuration ####

    # we need Boost.Algorithm, which does not need to be included explicitly since it's header-only
    # link to Boost::boost to include the header directories
    find_package(Boost 1.53.0 REQUIRED)

    ## XdgUtils
    if(USE_SYSTEM_XDGUTILS)
        find_package(XdgUtils REQUIRED COMPONENTS DesktopEntry BaseDir)
    else()
        message(STATUS "Downloading and building XdgUtils")

        ExternalProject_Add(
            XdgUtils-EXTERNAL
            GIT_REPOSITORY https://github.com/azubieta/xdg-utils-cxx.git
            GIT_TAG master
            GIT_SHALLOW On
            CMAKE_ARGS
            -DCMAKE_POSITION_INDEPENDENT_CODE=On
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
            -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}

            INSTALL_COMMAND ""
            UPDATE_DISCONNECTED On
        )

        import_external_project(
            TARGET_NAME XdgUtils::DesktopEntry
            EXT_PROJECT_NAME XdgUtils-EXTERNAL
            LIBRARIES "<BINARY_DIR>/src/DesktopEntry/libXdgUtilsDesktopEntry.a;"
            INCLUDE_DIRS "<SOURCE_DIR>/include"
        )

        import_external_project(
            TARGET_NAME XdgUtils::BaseDir
            EXT_PROJECT_NAME XdgUtils-EXTERNAL
            LIBRARIES "<BINARY_DIR>/src/BaseDir/libXdgUtilsBaseDir.a;"
            INCLUDE_DIRS "<SOURCE_DIR>/include"
        )
    endif()
endif()
