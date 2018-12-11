// libraries
#include <gtest/gtest.h>

// local
#include "utils/filesystem.h"


using namespace appimage::utils;

TEST(utils_filesystem_tests, parentPath) {
    ASSERT_EQ("/one/two", filesystem::parentPath("/one/two/three"));
    ASSERT_EQ("/one/two", filesystem::parentPath("/one/two//three"));
    ASSERT_EQ("/one/two", filesystem::parentPath("/one/two////three"));
    ASSERT_EQ("/", filesystem::parentPath("/one"));
    ASSERT_EQ(".", filesystem::parentPath("one"));
}
