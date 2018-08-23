#include <gtest/gtest.h>
#include <fstream>
#include <ftw.h>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

#include "fixtures.h"

extern "C" {
    #include <appimage/appimage_shared.h>
}


using namespace std;


// most simple derivative class for better naming of the tests in this file
class LibAppImageSharedTest : public TestBase {};


static bool test_strcmp(char* a, char* b) {
    return strcmp(a, b) == 0;
}

TEST_F(LibAppImageSharedTest, test_appimage_hexlify) {
    {
        char bytesIn[] = "\x00\x01\x02\x03\x04\x05\x06\x07";
        char expectedHex[] = "0001020304050607";

        char* hexlified = appimage_hexlify(bytesIn, 8);
        EXPECT_PRED2(test_strcmp, hexlified, expectedHex);

        // cleanup
        free(hexlified);
    }
    {
        char bytesIn[] = "\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";
        char expectedHex[] = "f8f9fafbfcfdfeff";

        char* hexlified = appimage_hexlify(bytesIn, 8);
        EXPECT_PRED2(test_strcmp, hexlified, expectedHex);

        // cleanup
        free(hexlified);
    }
}


bool isPowerOfTwo(int number) {
    return (number & (number - 1)) == 0;
}


TEST_F(LibAppImageSharedTest, test_appimage_get_elf_section_offset_and_length) {
    std::string appImagePath = std::string(TEST_DATA_DIR) + "/appimaged-i686.AppImage";

    unsigned long offset, length;

    ASSERT_TRUE(appimage_get_elf_section_offset_and_length(appImagePath.c_str(), ".upd_info", &offset, &length));

    EXPECT_GT(offset, 0);
    EXPECT_GT(length, 0);

    EXPECT_PRED1(isPowerOfTwo, length);
}


TEST_F(LibAppImageSharedTest, test_print_binary) {
    std::string appImagePath = std::string(TEST_DATA_DIR) + "/appimaged-i686.AppImage";

    unsigned long offset, length;

    ASSERT_TRUE(appimage_get_elf_section_offset_and_length(appImagePath.c_str(), ".upd_info", &offset, &length));

    EXPECT_EQ(appimage_print_binary(appImagePath.c_str(), offset, length), 0);
}


TEST_F(LibAppImageSharedTest, test_print_hex) {
    std::string appImagePath = std::string(TEST_DATA_DIR) + "/appimaged-i686.AppImage";

    unsigned long offset, length;

    ASSERT_TRUE(appimage_get_elf_section_offset_and_length(appImagePath.c_str(), ".sha256_sig", &offset, &length));

    EXPECT_EQ(appimage_print_hex(appImagePath.c_str(), offset, length), 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
