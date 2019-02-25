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

TEST(TestResourcesExtractor, extractDesktopEntry) {
    appimage::core::AppImage appImage(TEST_DATA_DIR "appimagetool-x86_64.AppImage");
    ResourcesExtractor extractor(appImage);

    DesktopEntry desktopEntry = extractor.extractDesktopEntry();

    ASSERT_EQ(static_cast<std::string>(desktopEntry["Desktop Entry/Name"]), "appimagetool");
}

TEST(TestResourcesExtractor, getIconPaths) {
    /* We need to edit the echo entry to properly tests this feature */

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
    extractor.extractEntriesTo(map);

    ASSERT_TRUE(bf::exists(tempFile));
    ASSERT_TRUE(bf::file_size(tempFile) > 0);

    bf::remove(tempFile);
}

TEST(TestResourcesExtractor, extractFile) {
    appimage::core::AppImage appImage(TEST_DATA_DIR "Echo-x86_64.AppImage");
    ResourcesExtractor extractor(appImage);

    auto fileData = extractor.extractFile("echo.desktop");

    ASSERT_FALSE(fileData.empty());
    ASSERT_THROW(extractor.extractFile("missing_file"), appimage::core::PayloadIteratorError);
}

TEST(TestResourcesExtractor, extractFiles) {
    appimage::core::AppImage appImage(TEST_DATA_DIR "Echo-x86_64.AppImage");
    ResourcesExtractor extractor(appImage);

    auto filesData = extractor.extractFiles({"echo.desktop", ".DirIcon"});

    ASSERT_FALSE(filesData.empty());
    for (const auto& itr: filesData)
        ASSERT_FALSE(itr.second.empty());

    ASSERT_THROW(extractor.extractFiles({"missing_file"}), appimage::core::PayloadIteratorError);
}
