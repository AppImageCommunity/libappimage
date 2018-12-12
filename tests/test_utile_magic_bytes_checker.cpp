// libraries
#include <gtest/gtest.h>

// local
#include "utils/magic_bytes_checker.h"

using namespace appimage::utils;

TEST(MagicBytesCheckerTests, hasIso9660Signature) {
    ASSERT_TRUE(magic_bytes_checker(TEST_DATA_DIR "/minimal.iso").hasIso9660Signature());
    ASSERT_TRUE(magic_bytes_checker(TEST_DATA_DIR "/AppImageExtract_6-x86_64.AppImage").hasIso9660Signature());
    ASSERT_FALSE(magic_bytes_checker(TEST_DATA_DIR "/Cura.desktop").hasIso9660Signature());
}

TEST(MagicBytesCheckerTests, hasElfSignature) {
    ASSERT_TRUE(magic_bytes_checker(TEST_DATA_DIR "/elffile").hasElfSignature());
    ASSERT_TRUE(magic_bytes_checker(TEST_DATA_DIR "/appimagetool-x86_64.AppImage").hasElfSignature());
    ASSERT_FALSE(magic_bytes_checker(TEST_DATA_DIR "/Cura.desktop").hasElfSignature());
}

TEST(MagicBytesCheckerTests, hasAppImageType1Signature) {
    ASSERT_TRUE(magic_bytes_checker(TEST_DATA_DIR "/AppImageExtract_6-x86_64.AppImage").hasAppImageType1Signature());
    ASSERT_FALSE(magic_bytes_checker(TEST_DATA_DIR "/appimagetool-x86_64.AppImage").hasAppImageType1Signature());
    ASSERT_FALSE(magic_bytes_checker(TEST_DATA_DIR "/elffile").hasAppImageType1Signature());
    ASSERT_FALSE(magic_bytes_checker(TEST_DATA_DIR "/Cura.desktop").hasAppImageType1Signature());
}

TEST(MagicBytesCheckerTests, hasAppImageType2Signature) {
    ASSERT_TRUE(magic_bytes_checker(TEST_DATA_DIR "/appimagetool-x86_64.AppImage").hasAppImageType2Signature());
    ASSERT_FALSE(magic_bytes_checker(TEST_DATA_DIR "/AppImageExtract_6-x86_64.AppImage").hasAppImageType2Signature());
    ASSERT_FALSE(magic_bytes_checker(TEST_DATA_DIR "/elffile").hasAppImageType2Signature());
    ASSERT_FALSE(magic_bytes_checker(TEST_DATA_DIR "/Cura.desktop").hasAppImageType2Signature());
}
