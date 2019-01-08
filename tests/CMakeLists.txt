# build and add test only if tests are enabled
include(CTest)
if(BUILD_TESTING)
    cmake_minimum_required(VERSION 3.5)

    # global definitions
    add_definitions(
        -DTEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/"
        -DGIT_COMMIT="AppImageKit unit tests"
    )

    add_library(fixtures INTERFACE)
    target_sources(fixtures INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fixtures.h)
    set_property(TARGET fixtures PROPERTY INTERFACE_LINK_LIBRARIES xdg-basedir gtest)

    add_executable(test_libappimage test_libappimage.cpp)
    target_link_libraries(test_libappimage fixtures libappimage libsquashfuse)
    add_test(test_libappimage test_libappimage)

    add_executable(test_shared test_shared.cpp)
    target_link_libraries(test_shared fixtures libappimage_shared)
    add_test(test_shared test_shared)
    # needed for some const->non-const conversions
    target_compile_options(test_shared PRIVATE -fpermissive)

    add_executable(test-xdg-basedir test-xdg-basedir.cpp)
    target_link_libraries(test-xdg-basedir fixtures xdg-basedir)
    add_test(test-xdg-basedir test-xdg-basedir)

    add_executable(test_desktop_integration test_desktop_integration.cpp file_management_utils.hpp)
    target_include_directories(test_desktop_integration PRIVATE "${PROJECT_SOURCE_DIR}/src/libappimage")
    target_link_libraries(test_desktop_integration libappimage libappimage_shared libsquashfuse gtest gtest_main)
    add_test(test_desktop_integration test_desktop_integration)

    add_subdirectory(libappimage)
    add_subdirectory(libappimage_desktop_integration)

    if(MEASURE_CODE_COVERAGE)
        set(COVERAGE_LCOV_EXCLUDES '${PROJECT_SOURCE_DIR}/lib/*' '${PROJECT_SOURCE_DIR}/tests/*' '${PROJECT_SOURCE_DIR}/*build*' '/usr/*')
        setup_target_for_coverage_lcov(
            NAME tests_code_coverage
            EXECUTABLE ctest -V -j ${PROCESSOR_COUNT}
            DEPENDENCIES test_libappimage_0.2 TestDesktopIntegration
        )
    endif()
endif()
