// system
#include <fstream>

// libraries
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

// local
#include "utils/IconHandle.h"

using namespace appimage::utils;
namespace bf = boost::filesystem;


TEST(TestUtilsIconHandle, loadFilePng) {
    IconHandle handle(TEST_DATA_DIR "squashfs-root/utilities-terminal.png");

    ASSERT_EQ(handle.format(), "png");
    ASSERT_EQ(handle.getSize(), 48);
}

TEST(TestUtilsIconHandle, savePngUnchanged) {
    IconHandle handle(TEST_DATA_DIR "squashfs-root/utilities-terminal.png");

    ASSERT_EQ(handle.format(), "png");
    ASSERT_EQ(handle.getSize(), 48);

    auto tmpPath = bf::temp_directory_path() / bf::unique_path();

    ASSERT_NO_THROW(handle.save(tmpPath.string(), "png"));
    ASSERT_TRUE(bf::exists(tmpPath));
    ASSERT_FALSE(bf::is_empty(tmpPath));

    bf::remove(tmpPath);
}

TEST(TestUtilsIconHandle, savePngResized) {
    IconHandle handle(TEST_DATA_DIR "squashfs-root/utilities-terminal.png");

    ASSERT_EQ(handle.format(), "png");
    ASSERT_EQ(handle.getSize(), 48);

    handle.setSize(256);

    auto tmpPath = bf::temp_directory_path() / bf::unique_path();

    ASSERT_NO_THROW(handle.save(tmpPath.string(), "png"));

    IconHandle handle2(tmpPath.string());
    ASSERT_EQ(handle2.format(), "png");
    ASSERT_EQ(handle2.getSize(), 256);

    bf::remove(tmpPath);
}

TEST(TestUtilsIconHandle, loadFileSvg) {
    IconHandle handle(TEST_DATA_DIR "squashfs-root/utilities-terminal.svg");

    ASSERT_EQ(handle.format(), "svg");
    ASSERT_EQ(handle.getSize(), 48);
}


TEST(TestUtilsIconHandle, saveSvg) {
    IconHandle handle(TEST_DATA_DIR "squashfs-root/utilities-terminal.svg");

    auto tmpPath = bf::temp_directory_path() / bf::unique_path();


    ASSERT_NO_THROW(handle.save(tmpPath.string(), "svg"));

    IconHandle handle2(tmpPath.string());
    ASSERT_EQ(handle2.format(), "svg");
    ASSERT_EQ(handle2.getSize(), 48);

    bf::remove(tmpPath);
}

TEST(TestUtilsIconHandle, saveSvgAsPng) {
    IconHandle handle(TEST_DATA_DIR "squashfs-root/utilities-terminal.svg");

    auto tmpPath = bf::temp_directory_path() / bf::unique_path();

    handle.setSize(256);
    ASSERT_NO_THROW(handle.save(tmpPath.string(), "png"));

    IconHandle handle2(tmpPath.string());
    ASSERT_EQ(handle2.format(), "png");
    ASSERT_EQ(handle2.getSize(), 256);

    bf::remove(tmpPath);
}
