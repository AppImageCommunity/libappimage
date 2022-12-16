// system
#include <fstream>

// libraries
#include <gtest/gtest.h>
#include <filesystem>

// local
#include "utils/IconHandle.h"
#include "TemporaryDirectory.h"

using namespace appimage::utils;


TEST(TestUtilsIconHandle, loadFilePng) {
    const IconHandle handle(TEST_DATA_DIR "squashfs-root/utilities-terminal.png");

    ASSERT_EQ(handle.format(), "png");
    ASSERT_EQ(handle.getSize(), 48);
}

TEST(TestUtilsIconHandle, savePngUnchanged) {
    const IconHandle handle(TEST_DATA_DIR "squashfs-root/utilities-terminal.png");

    ASSERT_EQ(handle.format(), "png");
    ASSERT_EQ(handle.getSize(), 48);

    const TemporaryDirectory tmpDir;
    const auto tmpFilePath = tmpDir.path() / "tempfile";

    ASSERT_NO_THROW(handle.save(tmpFilePath, "png"));
    ASSERT_TRUE(std::filesystem::exists(tmpDir.path()));
    ASSERT_FALSE(std::filesystem::is_empty(tmpDir.path()));
}

TEST(TestUtilsIconHandle, savePngResized) {
    IconHandle handle(TEST_DATA_DIR "squashfs-root/utilities-terminal.png");

    ASSERT_EQ(handle.format(), "png");
    ASSERT_EQ(handle.getSize(), 48);

    handle.setSize(256);

    const TemporaryDirectory tmpDir;
    const auto tmpFilePath = tmpDir.path() / "tempfile";

    ASSERT_NO_THROW(handle.save(tmpFilePath, "png"));

    const IconHandle handle2(tmpFilePath);
    ASSERT_EQ(handle2.format(), "png");
    ASSERT_EQ(handle2.getSize(), 256);
}

TEST(TestUtilsIconHandle, loadFileSvg) {
    const IconHandle handle(TEST_DATA_DIR "squashfs-root/utilities-terminal.svg");

    ASSERT_EQ(handle.format(), "svg");
    ASSERT_EQ(handle.getSize(), 48);
}


TEST(TestUtilsIconHandle, saveSvg) {
    const IconHandle handle(TEST_DATA_DIR "squashfs-root/utilities-terminal.svg");

    TemporaryDirectory tmpDir;
    const auto tmpFilePath = tmpDir.path() / "tempfile";

    ASSERT_NO_THROW(handle.save(tmpFilePath, "svg"));

    const IconHandle handle2(tmpFilePath);
    ASSERT_EQ(handle2.format(), "svg");
    ASSERT_EQ(handle2.getSize(), 48);
}

TEST(TestUtilsIconHandle, saveSvgAsPng) {
    IconHandle handle(TEST_DATA_DIR "squashfs-root/utilities-terminal.svg");

    TemporaryDirectory tmpDir;
    const auto tmpFilePath = tmpDir.path() / "tempfile";

    handle.setSize(256);
    ASSERT_NO_THROW(handle.save(tmpFilePath, "png"));

    const IconHandle handle2(tmpFilePath);
    ASSERT_EQ(handle2.format(), "png");
    ASSERT_EQ(handle2.getSize(), 256);
}
