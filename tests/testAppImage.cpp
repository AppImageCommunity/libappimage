// library headers
#include <gtest/gtest.h>
#include <vector>
#include <fstream>
#include <random>
#include <string>

#include <AppImageErrors.h>
#include "AppImage.h"

class AppImageTests : public ::testing::Test {
protected:

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

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

TEST_F(AppImageTests, getFormat) {
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR
                  "/AppImageExtract_6-x86_64.AppImage"), AppImage::Type1);
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR
                  "/AppImageExtract_6_no_magic_bytes-x86_64.AppImage"), AppImage::Type1);
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR
                  "/Echo-x86_64.AppImage"), AppImage::Type2);
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR
                  "/appimaged-i686.AppImage"), AppImage::Type2);
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR
                  "/elffile"), AppImage::Unknown);
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR
                  "/minimal.iso"), AppImage::Unknown);
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR
                  "/Cura.desktop"), AppImage::Unknown);
    ASSERT_EQ(AppImage::AppImage::getFormat(TEST_DATA_DIR
                  "/non_existend_file"), AppImage::Unknown);
}

TEST_F(AppImageTests, listType1Entries) {
    AppImage::AppImage appImage(TEST_DATA_DIR "/AppImageExtract_6-x86_64.AppImage");
    std::set<std::string> expected = {
        "usr",
        "usr/bin",
        "usr/lib",
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
    AppImage::AppImage appImage(TEST_DATA_DIR "/Echo-x86_64.AppImage");
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

    AppImage::AppImage appImage(TEST_DATA_DIR "/AppImageExtract_6-x86_64.AppImage");
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

    AppImage::AppImage appImage(TEST_DATA_DIR "/Echo-x86_64.AppImage");
    auto fItr = appImage.files().begin();
    while (fItr != fItr.end() && *fItr != "usr/share/applications/echo.desktop")
        ++fItr;
    std::cout << "Extracting " << *fItr << " to " << tmpFilePath << std::endl;
    fItr.extractTo(tmpFilePath);
    ASSERT_TRUE(fileSize(tmpFilePath) > 0);

    remove(tmpFilePath.c_str());
}

TEST_F(AppImageTests, type1ReadFile) {
    AppImage::AppImage appImage(TEST_DATA_DIR "/AppImageExtract_6-x86_64.AppImage");
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
    AppImage::AppImage appImage(TEST_DATA_DIR "/Echo-x86_64.AppImage");
    auto fItr = appImage.files().begin();
    std::vector<char> desktopData;
    std::vector<char> iconData;

    while (fItr != fItr.end()) {
        if (*fItr == "echo.desktop")
            desktopData.assign(std::istreambuf_iterator<char>(fItr.read()), std::istreambuf_iterator<char>());

        if (*fItr == "utilities-terminal.svg")
            iconData.assign(std::istreambuf_iterator<char>(fItr.read()), std::istreambuf_iterator<char>());
        ++fItr;
    }

    ASSERT_FALSE(desktopData.empty());
    ASSERT_FALSE(iconData.empty());
}
