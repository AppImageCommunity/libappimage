# build and add test only if tests are enabled
include(CTest)
if(BUILD_TESTING)
    cmake_minimum_required(VERSION 3.5)

    # global definitions
    add_definitions(
        -DTEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/"
        -DGIT_COMMIT="AppImageKit unit tests"
    )



    add_subdirectory(libappimage)

    if(MEASURE_CODE_COVERAGE)
        set(COVERAGE_LCOV_EXCLUDES '${PROJECT_SOURCE_DIR}/lib/*' '${PROJECT_SOURCE_DIR}/tests/*' '${PROJECT_SOURCE_DIR}/*build*' '/usr/*')
        setup_target_for_coverage_lcov(
            NAME tests_code_coverage
            EXECUTABLE ctest -V -j ${PROCESSOR_COUNT}
            DEPENDENCIES test_libappimage_0.2 TestDesktopIntegration
        )
    endif()
endif()
