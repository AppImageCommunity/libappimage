// system
#include <sstream>

// library headers
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include "integrator/ResourcesExtractor.h"

using namespace appimage::desktop_integration::integrator;
using namespace XdgUtils::DesktopEntry;
namespace bf = boost::filesystem;

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
    appimage::core::AppImage appImage(TEST_DATA_DIR "Echo-x86_64.AppImage");
    ResourcesExtractor extractor(appImage);

    auto tempFile = bf::temp_directory_path() / boost::filesystem::unique_path("libappimage-%%%%-%%%%-%%%%-%%%%");;
    std::map<std::string, std::string> map = {{".DirIcon", tempFile.string()}};
    extractor.extractTo(map);

    ASSERT_TRUE(bf::exists(tempFile));
    ASSERT_TRUE(bf::file_size(tempFile) > 0);

    bf::remove(tempFile);
}

TEST(TestResourcesExtractor, extractOne) {
    appimage::core::AppImage appImage(TEST_DATA_DIR "Echo-x86_64.AppImage");
    ResourcesExtractor extractor(appImage);

    auto fileData = extractor.extract("echo.desktop");

    ASSERT_FALSE(fileData.empty());
    ASSERT_THROW(extractor.extract("missing_file"), appimage::core::PayloadIteratorError);
}

TEST(TestResourcesExtractor, extractMany) {
    appimage::core::AppImage appImage(TEST_DATA_DIR "Echo-x86_64.AppImage");
    ResourcesExtractor extractor(appImage);

    auto filesData = extractor.extract(std::vector<std::string>{"echo.desktop", ".DirIcon"});

    ASSERT_FALSE(filesData.empty());
    for (const auto& itr: filesData)
        ASSERT_FALSE(itr.second.empty());

    ASSERT_THROW(extractor.extract(std::vector<std::string>{"missing_file"}), appimage::core::PayloadIteratorError);
}
