add_library(fixtures INTERFACE)
target_sources(fixtures INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fixtures.h)
set_property(TARGET fixtures PROPERTY INTERFACE_LINK_LIBRARIES xdg-basedir gtest)

if (NOT LIBAPPIMAGE_SHARED_ONLY)
    add_executable(test_libappimage test_libappimage.cpp)
    target_link_libraries(test_libappimage fixtures libappimage libsquashfuse libglib libgobject gtest gtest_main)

    add_executable(test-xdg-basedir test-xdg-basedir.cpp)
    target_link_libraries(test-xdg-basedir fixtures xdg-basedir)
    add_test(test-xdg-basedir test-xdg-basedir)
    add_test(test_libappimage test_libappimage)
endif()

add_executable(test_shared test_shared.cpp)
target_link_libraries(test_shared fixtures libappimage_shared)
add_test(test_shared test_shared)
# needed for some const->non-const conversions
target_compile_options(test_shared PRIVATE -fpermissive)
