// system
#include <vector>
#include <fstream>
#include <random>
#include <string>

// library
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

// local
#include <appimage/core/exceptions.h>
#include <appimage/core/AppImage.h>

using namespace appimage;
namespace bf = boost::filesystem;

class AppImageTests : public ::testing::Test {
protected:

    void SetUp() override {}

    void TearDown() override {}

    std::string random_string(std::string::size_type length) {
        static auto& chrs = "0123456789"
                            "abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        thread_local static std::mt19937 rg{std::random_device{}()};
        thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

        std::string s;

        s.reserve(length);

        while (length--)
            s += chrs[pick(rg)];

        return s;
    }

    std::string getTmpFilePath() {
        std::string tmpFilePath = "/tmp/libappimage-test-" + random_string(16);
        return tmpFilePath;
    }

    std::ifstream::pos_type fileSize(const std::string& filename) {
        std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
        return in.tellg();
    }
};

TEST_F(AppImageTests, instantiate) {
    ASSERT_NO_THROW(core::AppImage(TEST_DATA_DIR
                        "/AppImageExtract_6-x86_64.AppImage"));
    ASSERT_NO_THROW(core::AppImage(TEST_DATA_DIR
                        "/Echo-x86_64.AppImage"));
    ASSERT_THROW(core::AppImage(TEST_DATA_DIR
                     "/elffile"), core::AppImageError);
    ASSERT_THROW(core::AppImage(TEST_DATA_DIR
                     "/minimal.iso"), core::AppImageError);
    ASSERT_THROW(core::AppImage(TEST_DATA_DIR
                     "/Cura.desktop"), core::AppImageError);
    ASSERT_THROW(core::AppImage(TEST_DATA_DIR
                     "/none"), core::AppImageError);
}

TEST_F(AppImageTests, getFormat) {
    ASSERT_EQ(core::AppImage(TEST_DATA_DIR
                  "/AppImageExtract_6-x86_64.AppImage").getFormat(), core::AppImageFormat::TYPE_1);
    ASSERT_EQ(core::AppImage(TEST_DATA_DIR
                  "/AppImageExtract_6_no_magic_bytes-x86_64.AppImage").getFormat(), core::AppImageFormat::TYPE_1);
    ASSERT_EQ(core::AppImage(TEST_DATA_DIR
                  "/Echo-x86_64.AppImage").getFormat(), core::AppImageFormat::TYPE_2);
    ASSERT_EQ(core::AppImage(TEST_DATA_DIR
                  "/appimaged-i686.AppImage").getFormat(), core::AppImageFormat::TYPE_2);
    ASSERT_THROW(core::AppImage(TEST_DATA_DIR
                     "/elffile").getFormat(), core::AppImageError);
    ASSERT_THROW(core::AppImage(TEST_DATA_DIR
                     "/minimal.iso").getFormat(), core::AppImageError);
    ASSERT_THROW(core::AppImage(TEST_DATA_DIR
                     "/Cura.desktop").getFormat(), core::AppImageError);
    ASSERT_THROW(core::AppImage(TEST_DATA_DIR
                     "/non_existend_file").getFormat(), core::AppImageError);
}

TEST_F(AppImageTests, getPayloadOffset) {
    ASSERT_EQ(core::AppImage(TEST_DATA_DIR
                  "/AppImageExtract_6-x86_64.AppImage").getPayloadOffset(), 28040);
    ASSERT_EQ(core::AppImage(TEST_DATA_DIR
                  "/AppImageExtract_6_no_magic_bytes-x86_64.AppImage").getPayloadOffset(), 28040);
    ASSERT_EQ(core::AppImage(TEST_DATA_DIR
                  "/Echo-x86_64.AppImage").getPayloadOffset(), 187784);
    ASSERT_THROW(core::AppImage(TEST_DATA_DIR
                     "/elffile").getPayloadOffset(), core::AppImageError);
}

TEST_F(AppImageTests, listType1Entries) {
    core::AppImage appImage(TEST_DATA_DIR "/AppImageExtract_6-x86_64.AppImage");
    std::set<std::string> expected = {
        "AppImageExtract.desktop",
        ".DirIcon",
        "AppImageExtract.png",
        "usr/bin/appimageextract",
        "AppRun",
        "usr/bin/xorriso",
        "usr/lib/libburn.so.4",
        "usr/lib/libisoburn.so.1",
        "usr/lib/libisofs.so.6",
    };

    for (const auto& file: appImage.files())
        expected.erase(file);

    ASSERT_TRUE(expected.empty());
}

TEST_F(AppImageTests, listType2Entries) {
    core::AppImage appImage(TEST_DATA_DIR "/Echo-x86_64.AppImage");
    std::set<std::string> expected = {
        "echo.desktop",
        "AppRun",
        "usr",
        "usr/bin",
        "usr/bin/echo",
        "usr/share",
        "usr/share/applications",
        "usr/share/applications/echo.desktop",
        "usr/share/applications",
        "usr/share",
        "usr",
        "utilities-terminal.svg"
    };

    for (const auto& file: appImage.files())
        expected.erase(file);

    ASSERT_TRUE(expected.empty());
}

TEST_F(AppImageTests, type1ExtractFile) {
    auto tmpFilePath = getTmpFilePath();

    core::AppImage appImage(TEST_DATA_DIR "/AppImageExtract_6-x86_64.AppImage");
    auto fItr = appImage.files().begin();
    while (fItr != fItr.end() && *fItr != "AppImageExtract.desktop")
        ++fItr;
    std::cout << "Extracting " << *fItr << " to " << tmpFilePath << std::endl;
    fItr.extractTo(tmpFilePath);
    ASSERT_TRUE(fileSize(tmpFilePath) > 0);

    remove(tmpFilePath.c_str());
}

TEST_F(AppImageTests, type2ExtractFile) {
    auto tmpFilePath = getTmpFilePath();

    core::AppImage appImage(TEST_DATA_DIR "/Echo-x86_64.AppImage");
    auto fItr = appImage.files().begin();
    while (fItr != fItr.end() && *fItr != "usr/share/applications/echo.desktop")
        ++fItr;
    std::cout << "Extracting " << *fItr << " to " << tmpFilePath << std::endl;
    fItr.extractTo(tmpFilePath);
    ASSERT_TRUE(fileSize(tmpFilePath) > 0);

    remove(tmpFilePath.c_str());
}

TEST_F(AppImageTests, type1ReadFile) {
    core::AppImage appImage(TEST_DATA_DIR "/AppImageExtract_6-x86_64.AppImage");
    auto fItr = appImage.files().begin();
    std::vector<char> desktopData;
    std::vector<char> iconData;

    while (fItr != fItr.end()) {
        if (*fItr == "AppImageExtract.desktop")
            desktopData.assign(std::istreambuf_iterator<char>(fItr.read()), std::istreambuf_iterator<char>());

        if (*fItr == ".DirIcon")
            iconData.assign(std::istreambuf_iterator<char>(fItr.read()), std::istreambuf_iterator<char>());
        ++fItr;
    }

    ASSERT_FALSE(desktopData.empty());
    ASSERT_FALSE(iconData.empty());
}

TEST_F(AppImageTests, type2ReadFile) {
    core::AppImage appImage(TEST_DATA_DIR "/Echo-x86_64.AppImage");
    auto fItr = appImage.files().begin();
    std::vector<char> desktopData;
    std::vector<char> iconData;

    while (fItr != fItr.end()) {
        if (*fItr == "usr/share/applications/echo.desktop")
            desktopData.assign(std::istreambuf_iterator<char>(fItr.read()), std::istreambuf_iterator<char>());

        if (*fItr == "utilities-terminal.svg")
            iconData.assign(std::istreambuf_iterator<char>(fItr.read()), std::istreambuf_iterator<char>());
        ++fItr;
    }

    ASSERT_FALSE(desktopData.empty());
    ASSERT_FALSE(iconData.empty());
}

TEST_F(AppImageTests, extractEntryMultipleTimes) {
    auto tmpPath = bf::temp_directory_path() / bf::unique_path();

    core::AppImage appImage(TEST_DATA_DIR "/Echo-x86_64.AppImage");
    auto itr = appImage.files();
    // Extract two times

    ASSERT_NO_THROW(itr.extractTo(tmpPath.string()));
    ASSERT_THROW(itr.extractTo(tmpPath.string()), core::PayloadIteratorError);
    bf::remove(tmpPath);

    // Extract and read
    itr = appImage.files();
    ASSERT_NO_THROW(itr.extractTo(tmpPath.string()));
    ASSERT_THROW(itr.read(), core::PayloadIteratorError);
    bf::remove(tmpPath);

    // Read two times
    itr = appImage.files();
    ASSERT_NO_THROW(std::string(std::istream_iterator<char>(itr.read()), std::istream_iterator<char>()));
    ASSERT_THROW(itr.read(), core::PayloadIteratorError);

    // Read and extract
    itr = appImage.files();
    ASSERT_NO_THROW(std::string(std::istream_iterator<char>(itr.read()), std::istream_iterator<char>()));
    ASSERT_THROW(itr.extractTo(tmpPath.string()), core::PayloadIteratorError);
    bf::remove(tmpPath);
}
