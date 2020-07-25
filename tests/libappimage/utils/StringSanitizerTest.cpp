// library includes
#include <gtest/gtest.h>

// local includes
#include "utils/StringSanitizer.h"

class StringSanitizerTest : public ::testing::Test {};

TEST_F(StringSanitizerTest, testSanitizeForPathWithEmptyPath) {
    const auto actual = StringSanitizer("").sanitizeForPath();
    EXPECT_TRUE(actual.empty());
}

TEST_F(StringSanitizerTest, testSanitizeForPathWithAlreadySafeString) {
    const std::string alreadySafeString = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.";
    EXPECT_TRUE(alreadySafeString == StringSanitizer(alreadySafeString).sanitizeForPath());
}

TEST_F(StringSanitizerTest, testSanitizeForPathWithUnsafePath) {
    const std::string unsafeString = "/../$§%&testabcdefg";
    const std::string expected = "_..______testabcdefg";
    const auto actual = StringSanitizer(unsafeString).sanitizeForPath();
    EXPECT_TRUE(expected == actual);
}

TEST_F(StringSanitizerTest, testSanitizeForPathWithSpaces) {
    const std::string unsafeString = "test string abcdefg hijklmnop ";
    const std::string expected = "test_string_abcdefg_hijklmnop_";
    const auto actual = StringSanitizer(unsafeString).sanitizeForPath();
    EXPECT_TRUE(expected == actual);
}
