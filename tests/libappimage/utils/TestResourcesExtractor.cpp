// system
#include <sstream>

// library headers
#include <gtest/gtest.h>
#include <filesystem>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include <appimage/utils/ResourcesExtractor.h>
#include "TemporaryDirectory.h"

using namespace appimage::utils;
using namespace XdgUtils::DesktopEntry;

TEST(TestResourcesExtractor, getDesktopEntryPath) {
    appimage::core::AppImage appImage(TEST_DATA_DIR "appimagetool-x86_64.AppImage");
    ResourcesExtractor extractor(appImage);

    auto desktopEntryPath = extractor.getDesktopEntryPath();

    ASSERT_EQ(desktopEntryPath, "appimagetool.desktop");
}

TEST(TestResourcesExtractor, getIconPaths) {
    /* We need to edit the echo AppImage to properly tests this feature */

//    appimage::core::AppImage appImage(TEST_DATA_DIR "Echo-x86_64.AppImage");
//    ResourcesExtractor extractor(appImage);
//
//    auto iconFilePaths = extractor.getIconFilePaths("utilities-terminal");
//
//    const std::vector<std::string> expected = {"usr/share/icons/hicolor/scalable/utilities-terminal.svg"};
//    ASSERT_EQ(iconFilePaths,  expected);
}

TEST(TestResourcesExtractor, extractEntriesTo) {
    const appimage::core::AppImage appImage(TEST_DATA_DIR "Echo-x86_64.AppImage");
    const ResourcesExtractor extractor(appImage);

    const TemporaryDirectory tmpDir;
    const auto tmpFilePath = tmpDir.path() / "libappimage-0000-0000-0000-0000";

    const std::map<std::string, std::string> map = {{".DirIcon", tmpFilePath}};
    extractor.extractTo(map);

    ASSERT_TRUE(std::filesystem::exists(tmpFilePath));
    ASSERT_TRUE(std::filesystem::file_size(tmpFilePath) > 0);
}

TEST(TestResourcesExtractor, extractOne) {
    const appimage::core::AppImage appImage(TEST_DATA_DIR "Echo-x86_64.AppImage");
    const ResourcesExtractor extractor(appImage);

    const auto fileData = extractor.extract("echo.desktop");

    ASSERT_FALSE(fileData.empty());
    ASSERT_THROW(extractor.extract("missing_file"), appimage::core::PayloadIteratorError);
}

TEST(TestResourcesExtractor, extractMany) {
    const appimage::core::AppImage appImage(TEST_DATA_DIR "Echo-x86_64.AppImage");
    const ResourcesExtractor extractor(appImage);

    const auto filesData = extractor.extract(std::vector<std::string>{"echo.desktop", ".DirIcon"});

    ASSERT_FALSE(filesData.empty());

    ASSERT_TRUE(std::all_of(filesData.begin(), filesData.end(), [](auto entry) {
        return !entry.second.empty();
    }));

    ASSERT_THROW(extractor.extract(std::vector<std::string>{"missing_file"}), appimage::core::PayloadIteratorError);
}
