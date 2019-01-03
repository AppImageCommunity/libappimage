// libraries
#include <gtest/gtest.h>

// local
#include "utils/Elf.h"

using namespace appimage::utils;

TEST(TestUtilsElf, getSize) {
    ASSERT_EQ(Elf(TEST_DATA_DIR "/AppImageExtract_6-x86_64.AppImage").getSize(), 28040);
    ASSERT_EQ(Elf(TEST_DATA_DIR "/AppImageExtract_6_no_magic_bytes-x86_64.AppImage").getSize(), 28040);
    ASSERT_EQ(Elf(TEST_DATA_DIR "/Echo-x86_64.AppImage").getSize(), 187784);
    ASSERT_EQ(Elf(TEST_DATA_DIR "/appimaged-i686.AppImage").getSize(), 91148);
}
