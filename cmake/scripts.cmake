cmake_minimum_required(VERSION 3.2)

include(ExternalProject)

# searches for absolute paths of libraries, applying LIBRARY_DIRS
# CMake prefers absolute paths of libraries in non-standard locations, apparently
# see FindPkgConfig.cmake's _pkg_find_libs for more information
#
# positional parameters:
#  - libraries: name of variable containing list of libraries
#  - library_dirs
function(apply_library_dirs libraries library_dirs)
    foreach(library ${${libraries}})
        find_library(${library}_path ${library} PATHS ${${library_dirs}} NO_DEFAULT_PATH)
        if(NOT ${library}_path)
            list(APPEND new_libraries ${library})
        else()
            list(APPEND new_libraries ${${library}_path})
        endif()
    endforeach()
    set(${libraries} ${new_libraries} PARENT_SCOPE)
    unset(new_libraries)
endfunction()

# imports a library from the standard set of variables CMake creates when using its pkg-config module or find_package
# this is code shared by import_pkgconfig_target and import_external_project, hence it's been extracted into a separate
# CMake function
#
# partly inspired by https://github.com/Kitware/CMake/blob/master/Modules/FindPkgConfig.cmake#L187
#
# positional parameters:
#  - target_name: name of the target that should be created
#  - variable_prefix: prefix of the variable that should be used to create the target from
function(import_library_from_prefix target_name variable_prefix)
    if(TARGET ${target_name})
        message(WARNING "Target exists already, skipping")
        return()
    endif()

    add_library(${target_name} INTERFACE IMPORTED GLOBAL)

    if(${variable_prefix}_INCLUDE_DIRS)
        # need to create directories before setting INTERFACE_INCLUDE_DIRECTORIES, otherwise CMake will complain
        # possibly related: https://cmake.org/Bug/view.php?id=15052
        foreach(dir ${${variable_prefix}_INCLUDE_DIRS})
            if(NOT EXISTS ${dir})
                if (${dir} MATCHES ${CMAKE_BINARY_DIR})
                    file(MAKE_DIRECTORY ${dir})
                    list(APPEND include_dirs ${dir})
                endif()
            else()
                list(APPEND include_dirs ${dir})
            endif()
        endforeach()
        set_property(TARGET ${target_name} PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${include_dirs})
        unset(include_dirs)
    endif()

    # if library dirs are set, figure out absolute paths to libraries, like CMake's FindPkgConfig module does
    if(${variable_prefix}_LIBRARY_DIRS)
        apply_library_dirs(${variable_prefix}_LIBRARIES ${variable_prefix}_LIBRARY_DIRS)
    endif()

    set_property(TARGET ${target_name} PROPERTY INTERFACE_LINK_LIBRARIES ${${variable_prefix}_LIBRARIES})

    if(${variable_prefix}_CFLAGS_OTHER)
        set_property(TARGET ${target_name} PROPERTY INTERFACE_COMPILE_OPTIONS ${${variable_prefix}_CFLAGS_OTHER})
    endif()

    # export some of the imported properties with the target name as prefix
    # this is necessary to allow the other external projects, which are not built with CMake or not within the same
    # CMake context, to link to the libraries built as external projects (or the system ones, depending on the build
    # configuration)
    set(${target_name}_INCLUDE_DIRS ${${variable_prefix}_INCLUDE_DIRS} CACHE INTERNAL "")
    set(${target_name}_LIBRARIES ${${variable_prefix}_LIBRARIES} CACHE INTERNAL "")
    set(${target_name}_LIBRARY_DIRS ${${variable_prefix}_LIBRARY_DIRS} CACHE INTERNAL "")
    # TODO: the following might not always apply
    set(${target_name}_PREFIX ${CMAKE_INSTALL_PREFIX}/lib CACHE INTERNAL "")
endfunction()


# imports a library using pkg-config
#
# positional parameters:
#  - target_name: name of the target that we shall create for you
#  - pkg_config_target: librar(y name to pass to pkg-config (may include a version)
function(import_pkgconfig_target)
    set(keywords STATIC)
    set(oneValueArgs TARGET_NAME PKGCONFIG_TARGET)
    cmake_parse_arguments(IMPORT_PKGCONFIG_TARGET "${keywords}" "${oneValueArgs}" "" "${ARGN}")

    # check whether parameters have been set
    if(NOT IMPORT_PKGCONFIG_TARGET_TARGET_NAME)
        message(FATAL_ERROR "TARGET_NAME parameter missing, but is required")
    endif()
    if(NOT IMPORT_PKGCONFIG_TARGET_PKGCONFIG_TARGET)
        message(FATAL_ERROR "PKGCONFIG_TARGET parameter missing, but is required")
    endif()

    find_package(PkgConfig REQUIRED)

    set(type "shared")
    if(IMPORT_PKGCONFIG_TARGET_STATIC)
        set(type "static")
    endif()

    message(STATUS "Importing target ${IMPORT_PKGCONFIG_TARGET_TARGET_NAME} via pkg-config (${IMPORT_PKGCONFIG_TARGET_PKGCONFIG_TARGET}, ${type})")

    pkg_check_modules(${IMPORT_PKGCONFIG_TARGET_TARGET_NAME}-IMPORTED REQUIRED ${IMPORT_PKGCONFIG_TARGET_PKGCONFIG_TARGET})

    if(IMPORT_PKGCONFIG_TARGET_STATIC)
        set(prefix ${IMPORT_PKGCONFIG_TARGET_TARGET_NAME}-IMPORTED_STATIC)
    else()
        set(prefix ${IMPORT_PKGCONFIG_TARGET_TARGET_NAME}-IMPORTED)
    endif()

    import_library_from_prefix(${IMPORT_PKGCONFIG_TARGET_TARGET_NAME} ${prefix})
endfunction()

function(import_find_pkg_target target_name pkg_name variable_prefix)
    message(STATUS "Importing target ${target_name} via find_package (${pkg_name})")

    find_package(${pkg_name})
    if(NOT ${variable_prefix}_FOUND)
        message(FATAL_ERROR "${pkg_name} could not be found on the system. You will have to either install it, or use the bundled package.")
    endif()

    import_library_from_prefix(${target_name} ${variable_prefix})
endfunction()


# imports a library from an existing external project
#
# required parameters:
#  - TARGET_NAME:
function(import_external_project)
    set(oneValueArgs TARGET_NAME EXT_PROJECT_NAME)
    set(multiValueArgs LIBRARIES INCLUDE_DIRS LIBRARY_DIRS)
    cmake_parse_arguments(IMPORT_EXTERNAL_PROJECT "" "${oneValueArgs}" "${multiValueArgs}" "${ARGN}")

    # check whether parameters have been set
    if(NOT IMPORT_EXTERNAL_PROJECT_TARGET_NAME)
        message(FATAL_ERROR "TARGET_NAME parameter missing, but is required")
    endif()
    if(NOT IMPORT_EXTERNAL_PROJECT_EXT_PROJECT_NAME)
        message(FATAL_ERROR "EXT_PROJECT_NAME parameter missing, but is required")
    endif()
    if(NOT IMPORT_EXTERNAL_PROJECT_LIBRARIES)
        message(FATAL_ERROR "LIBRARIES parameter missing, but is required")
    endif()
    if(NOT IMPORT_EXTERNAL_PROJECT_INCLUDE_DIRS)
        message(FATAL_ERROR "INCLUDE_DIRS parameter missing, but is required")
    endif()

    if(TARGET ${target_name})
        message(WARNING "Target exists already, skipping")
        return()
    endif()

    add_library(${IMPORT_EXTERNAL_PROJECT_TARGET_NAME} INTERFACE IMPORTED GLOBAL)

    ExternalProject_Get_Property(${IMPORT_EXTERNAL_PROJECT_EXT_PROJECT_NAME} SOURCE_DIR)
    ExternalProject_Get_Property(${IMPORT_EXTERNAL_PROJECT_EXT_PROJECT_NAME} INSTALL_DIR)
    ExternalProject_Get_Property(${IMPORT_EXTERNAL_PROJECT_EXT_PROJECT_NAME} BINARY_DIR)

    # "evaluate" patterns in the passed arguments by using some string replacing magic
    # this makes it easier to use this function, as some external project properties don't need to be evaluated and
    # passed beforehand, and should reduce the amount of duplicate code in this file
    foreach(item ITEMS
        IMPORT_EXTERNAL_PROJECT_EXT_PROJECT_NAME
        IMPORT_EXTERNAL_PROJECT_LIBRARIES
        IMPORT_EXTERNAL_PROJECT_INCLUDE_DIRS
        IMPORT_EXTERNAL_PROJECT_LIBRARY_DIRS)

        # create new variable with fixed string...
        string(REPLACE "<SOURCE_DIR>" "${SOURCE_DIR}" ${item}-out "${${item}}")
        # ... and set the original value to the new value
        set(${item} "${${item}-out}")

        # create new variable with fixed string...
        string(REPLACE "<INSTALL_DIR>" "${INSTALL_DIR}" ${item}-out "${${item}}")
        # ... and set the original value to the new value
        set(${item} "${${item}-out}")

        # create new variable with fixed string...
        string(REPLACE "<BINARY_DIR>" "${BINARY_DIR}" ${item}-out "${${item}}")
        # ... and set the original value to the new value
        set(${item} "${${item}-out}")
    endforeach()

    # if library dirs are set, figure out absolute paths to libraries, like CMake's FindPkgConfig module does
    if(${IMPORT_EXTERNAL_PROJECT_LIBRARY_DIRS})
        apply_library_dirs(IMPORT_EXTERNAL_PROJECT_LIBRARIES IMPORT_EXTERNAL_PROJECT_LIBRARY_DIRS)
    endif()

    set_property(TARGET ${IMPORT_EXTERNAL_PROJECT_TARGET_NAME} PROPERTY INTERFACE_LINK_LIBRARIES "${IMPORT_EXTERNAL_PROJECT_LIBRARIES}")

    if(IMPORT_EXTERNAL_PROJECT_INCLUDE_DIRS)
        # need to create directories before setting INTERFACE_INCLUDE_DIRECTORIES, otherwise CMake will complain
        # possibly related: https://cmake.org/Bug/view.php?id=15052

        foreach(dir ${IMPORT_EXTERNAL_PROJECT_INCLUDE_DIRS})
            if(NOT EXISTS ${dir})
                if (${dir} MATCHES ${CMAKE_BINARY_DIR})
                    file(MAKE_DIRECTORY ${dir})
                    list(APPEND include_dirs ${dir})
                endif()
            else()
                list(APPEND include_dirs ${dir})
            endif()
        endforeach()
        set_property(TARGET ${IMPORT_EXTERNAL_PROJECT_TARGET_NAME} PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${include_dirs})
        unset(include_dirs)
    endif()

    # finally, add a depenceny on the external project to make sure it's built
    add_dependencies(${IMPORT_EXTERNAL_PROJECT_TARGET_NAME} "${IMPORT_EXTERNAL_PROJECT_EXT_PROJECT_NAME}")

    # read external project properties, and export them with the target name as prefix
    # this is necessary to allow the other external projects, which are not built with CMake or not within the same
    # CMake context, to link to the libraries built as external projects (or the system ones, depending on the build
    # configuration)
    set(${IMPORT_EXTERNAL_PROJECT_TARGET_NAME}_INCLUDE_DIRS "${IMPORT_EXTERNAL_PROJECT_INCLUDE_DIRS}" CACHE INTERNAL "")
    set(${IMPORT_EXTERNAL_PROJECT_TARGET_NAME}_LIBRARIES "${IMPORT_EXTERNAL_PROJECT_LIBRARIES}" CACHE INTERNAL "")
    set(${IMPORT_EXTERNAL_PROJECT_TARGET_NAME}_LIBRARY_DIRS "${IMPORT_EXTERNAL_PROJECT_LIBRARY_DIRS}" CACHE INTERNAL "")
    set(${IMPORT_EXTERNAL_PROJECT_TARGET_NAME}_PREFIX ${INSTALL_DIR} CACHE INTERNAL "")
endfunction()

# @brief Configure a libappimage module by setting
#
# Sets set to the given <target> the public headers, the compile definitions and the include directories. Which are
# common to all modules.
function(configure_libappimage_module target)
    # targets are called lib* already, so CMake shouldn't add another lib prefix to the actual files
    set_target_properties(${target}
        PROPERTIES PREFIX ""
        PUBLIC_HEADER ${libappimage_public_header}
        POSITION_INDEPENDENT_CODE ON
    )

    target_compile_definitions(${target}
        # Support Large Files
        PRIVATE -D_FILE_OFFSET_BITS=64
        PRIVATE -D_LARGEFILE_SOURCE

        PRIVATE -DGIT_COMMIT="${GIT_COMMIT}"
        PRIVATE -DENABLE_BINRELOC
    )

    target_include_directories(${target}
        PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
        PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/libappimage>
        INTERFACE $<INSTALL_INTERFACE:include/>
    )
endfunction()
