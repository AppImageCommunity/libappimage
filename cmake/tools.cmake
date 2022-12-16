if(TOOLS_PREFIX)
    message(STATUS "TOOLS_PREFIX set, looking for programs with prefix ${TOOLS_PREFIX}")
endif()

# first of all, make sure required programs are available
function(check_program)
    set(keywords FORCE_PREFIX)
    set(oneValueArgs NAME)
    cmake_parse_arguments(ARG "${keywords}" "${oneValueArgs}" "" "${ARGN}")

    if(NOT ARG_NAME)
        message(FATAL_ERROR "NAME argument required for check_program")
    endif()

    if(TOOLS_PREFIX)
        set(prefix ${TOOLS_PREFIX})
    endif()

    message(STATUS "Checking for program ${ARG_NAME}")

    string(TOUPPER ${ARG_NAME} name_upper)

    if(prefix)
        # try prefixed version first
        find_program(${name_upper} ${prefix}${ARG_NAME})
    endif()

    # try non-prefixed version
    if(NOT ${name_upper})
        if(TOOLS_PREFIX AND ARG_FORCE_PREFIX)
            message(FATAL_ERROR "TOOLS_PREFIX set, but could not find program with prefix in PATH (FORCE_PREFIX is set)")
        endif()

        find_program(${name_upper} ${ARG_NAME})

        if(NOT ${name_upper})
            message(FATAL_ERROR "Could not find required program ${ARG_NAME}.")
        endif()
    endif()

    message(STATUS "Found program ${ARG_NAME}: ${${name_upper}}")

    mark_as_advanced(${name_upper})
endfunction()

if (NOT LIBAPPIMAGE_SHARED_ONLY)
    check_program(NAME aclocal)
    check_program(NAME autoheader)
    check_program(NAME automake)
    check_program(NAME autoreconf)
    check_program(NAME libtoolize)
    check_program(NAME patch)
    check_program(NAME sed)
    check_program(NAME make)
endif()
if(BUILD_TESTING)
    check_program(NAME desktop-file-validate)
endif()
