# >= 3.2 required for ExternalProject_Add_StepDependencies
cmake_minimum_required(VERSION 3.2)

include(${CMAKE_CURRENT_LIST_DIR}/scripts.cmake)

# imported dependencies
include(${CMAKE_CURRENT_LIST_DIR}/imported_dependencies.cmake)

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


set(USE_SYSTEM_XZ OFF CACHE BOOL "Use system xz/liblzma instead of building our own")

if(NOT USE_SYSTEM_XZ)
    message(STATUS "Downloading and building xz")

    ExternalProject_Add(
        xz-EXTERNAL
        URL https://netcologne.dl.sourceforge.net/project/lzmautils/xz-5.2.5.tar.gz
        URL_HASH SHA512=7443674247deda2935220fbc4dfc7665e5bb5a260be8ad858c8bd7d7b9f0f868f04ea45e62eb17c0a5e6a2de7c7500ad2d201e2d668c48ca29bd9eea5a73a3ce
        CONFIGURE_COMMAND CC=${CC} CXX=${CXX} CFLAGS=${CFLAGS} CPPFLAGS=${CPPFLAGS} LDFLAGS=${LDFLAGS} <SOURCE_DIR>/configure --with-pic --disable-shared --enable-static --prefix=<INSTALL_DIR> --libdir=<INSTALL_DIR>/lib ${EXTRA_CONFIGURE_FLAGS} --disable-xz --disable-xzdec
        BUILD_COMMAND ${MAKE}
        INSTALL_COMMAND ${MAKE} install
        UPDATE_DISCONNECTED On
    )

    import_external_project(
        TARGET_NAME xz
        EXT_PROJECT_NAME xz-EXTERNAL
        LIBRARY_DIRS <INSTALL_DIR>/lib/
        LIBRARIES "<INSTALL_DIR>/lib/liblzma.a"
        INCLUDE_DIRS "<SOURCE_DIR>/src/liblzma/api/"
    )
else()
    message(STATUS "Using system xz")

    import_pkgconfig_target(TARGET_NAME xz PKGCONFIG_TARGET liblzma)
endif()


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


set(USE_SYSTEM_LIBARCHIVE OFF CACHE BOOL "Use system libarchive instead of building our own")

if(NOT USE_SYSTEM_LIBARCHIVE)
    message(STATUS "Downloading and building libarchive")

    ExternalProject_Add(
        libarchive-EXTERNAL
        URL https://www.libarchive.org/downloads/libarchive-3.3.1.tar.gz
        URL_HASH SHA512=90702b393b6f0943f42438e277b257af45eee4fa82420431f6a4f5f48bb846f2a72c8ff084dc3ee9c87bdf8b57f4d8dddf7814870fe2604fe86c55d8d744c164
        CONFIGURE_COMMAND CC=${CC} CXX=${CXX} CFLAGS=${CFLAGS} CPPFLAGS=${CPPFLAGS} LDFLAGS=${LDFLAGS} <SOURCE_DIR>/configure --with-pic --disable-shared --enable-static --disable-bsdtar --disable-bsdcat --disable-bsdcpio --with-zlib --without-bz2lib --without-iconv --without-lz4 --without-lzma --without-lzo2 --without-nettle --without-openssl --without-xml2 --without-expat --prefix=<INSTALL_DIR> --libdir=<INSTALL_DIR>/lib ${EXTRA_CONFIGURE_FLAGS}
        BUILD_COMMAND ${MAKE}
        INSTALL_COMMAND ${MAKE} install
        UPDATE_DISCONNECTED On
    )

    import_external_project(
        TARGET_NAME libarchive
        EXT_PROJECT_NAME libarchive-EXTERNAL
        LIBRARIES "<INSTALL_DIR>/lib/libarchive.a"
        INCLUDE_DIRS "<INSTALL_DIR>/include/"
    )
else()
    message(STATUS "Using system libarchive")

    import_find_pkg_target(libarchive LibArchive LibArchive)
endif()


#### build dependency configuration ####

# only have to build custom xz when not using system libxz
if(TARGET xz-EXTERNAL)
    if(TARGET squashfuse-EXTERNAL)
        ExternalProject_Add_StepDependencies(squashfuse-EXTERNAL configure xz-EXTERNAL)
    endif()
endif()

## Boost
if(NOT USE_SYSTEM_BOOST)
    message(STATUS "Downloading and building boost")

    if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i386")
        set(BOOST_B2_TARGET_CONFIG architecture=x86 address-model=32)
    endif()

    # support for clang compiler
    # if the toolset is not explicitly specified, ./bootstrap.sh will not generate the ./b2 script
    string(TOLOWER ${CMAKE_CXX_COMPILER_ID} boost_compiler_id)

    # of course, there has to be some exception to that snippet
    # CMake's "gnu" toolset is called "gcc" in boost
    if(${boost_compiler_id} STREQUAL gnu)
        set(boost_compiler_id gcc)
    endif()

    ExternalProject_Add(
        boost-EXTERNAL
        URL https://downloads.sourceforge.net/project/boost/boost/1.69.0/boost_1_69_0.tar.gz
        URL_HASH SHA256=9a2c2819310839ea373f42d69e733c339b4e9a19deab6bfec448281554aa4dbb
        CONFIGURE_COMMAND ./bootstrap.sh --with-libraries=filesystem,system,thread --with-toolset=${boost_compiler_id}
        BUILD_COMMAND ./b2 ${BOOST_B2_TARGET_CONFIG} cxxflags=-fPIC ${CPPFLAGS} cflags=-fPIC ${CFLAGS} link=static
        INSTALL_COMMAND ""
        BUILD_IN_SOURCE 1
        UPDATE_DISCONNECTED On
    )

    import_external_project(
        TARGET_NAME Boost::filesystem
        EXT_PROJECT_NAME boost-EXTERNAL
        LIBRARIES "<BINARY_DIR>/stage/lib/libboost_filesystem.a;<BINARY_DIR>/stage/lib/libboost_system.a"
        INCLUDE_DIRS "<BINARY_DIR>"
    )

else()
    find_package(Boost REQUIRED COMPONENTS filesystem)
endif()


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
