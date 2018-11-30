// library headers
#include <gtest/gtest.h>
#include <Errors.h>

#include "AppImage.h"

class AppImageTests : public ::testing::Test {
    protected:

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F(AppImageTests, getFormat) {
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR "/AppImageExtract_6-x86_64.AppImage"), AppImage::Type1);
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR "/AppImageExtract_6_no_magic_bytes-x86_64.AppImage"), AppImage::Type1);
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR "/Echo-x86_64.AppImage"), AppImage::Type2);
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR "/appimaged-i686.AppImage"), AppImage::Type2);
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR "/elffile"), AppImage::Unknown);
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR "/minimal.iso"), AppImage::Unknown);
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR "/Cura.desktop"), AppImage::Unknown);
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR "/non_existend_file"), AppImage::Unknown);
}

