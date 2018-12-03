// library headers
#include <gtest/gtest.h>
#include <AppImageErrors.h>

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

TEST_F(AppImageTests, listType1Entries) {
    AppImage::AppImage appImage(TEST_DATA_DIR "/AppImageExtract_6-x86_64.AppImage");
    std::set<std::string> expected = {
        ".",
        "./usr",
        "./usr/bin",
        "./usr/lib",
        "./AppImageExtract.desktop",
        "./.DirIcon",
        "./AppImageExtract.png",
        "./usr/bin/appimageextract",
        "./AppRun",
        "./usr/bin/xorriso",
        "./usr/lib/libburn.so.4",
        "./usr/lib/libisoburn.so.1",
        "./usr/lib/libisofs.so.6",
    };

    for (const auto &file: appImage.files())
        expected.erase(file);

    ASSERT_TRUE(expected.empty());
}

TEST_F(AppImageTests, listType2Entries) {
    AppImage::AppImage appImage(TEST_DATA_DIR "/Echo-x86_64.AppImage");
    std::set<std::string> expected = {
        ".",
        "./usr",
        "./usr/bin",
        "./usr/lib",
        "./AppImageExtract.desktop",
        "./.DirIcon",
        "./AppImageExtract.png",
        "./usr/bin/appimageextract",
        "./AppRun",
        "./usr/bin/xorriso",
        "./usr/lib/libburn.so.4",
        "./usr/lib/libisoburn.so.1",
        "./usr/lib/libisofs.so.6",
    };

    for (const auto &file: appImage.files())
        expected.erase(file);

    ASSERT_TRUE(expected.empty());
}
