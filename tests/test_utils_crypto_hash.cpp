#include <gtest/gtest.h>
#include "utils/md5sum.h"

using namespace appimage::utils;

TEST(utils_md5sum_tests, md5) {
    md5sum md5("/this/is/a/random/path.AppImage");
    ASSERT_EQ(md5.hexdigest(), "cdfd9eac8b163febbf9a061eed9d488d");
}
