// system
#include <sstream>

// library headers
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

// local
#include "utils/resources_extractor/PayloadEntriesCache.h"

using namespace appimage::utils;

class TestPayloadEntriesCache : public ::testing::Test {
protected:
    std::shared_ptr<appimage::core::AppImage> appImage;
    std::shared_ptr<PayloadEntriesCache> entriesCache;

    void SetUp() override {
        appImage = std::make_shared<appimage::core::AppImage>(TEST_DATA_DIR "appimagetool-x86_64.AppImage");
        entriesCache = std::make_shared<PayloadEntriesCache>(*appImage);
    }
};

TEST_F(TestPayloadEntriesCache, getEntriesPath) {
    auto paths = entriesCache->getEntriesPaths();
    std::vector<std::string> expectedPaths = {
        ".DirIcon", "AppRun", "appimagetool.desktop", "appimagetool.svg", "usr", "usr/bin", "usr/bin/appimagetool",
        "usr/bin/desktop-file-validate", "usr/bin/file", "usr/bin/zsyncmake", "usr/lib", "usr/lib/appimagekit",
        "usr/lib/appimagekit/mksquashfs", "usr/share", "usr/share/metainfo",
        "usr/share/metainfo/appimagetool.appdata.xml",
    };

    ASSERT_EQ(paths, expectedPaths);
}

TEST_F(TestPayloadEntriesCache, getEntryTypeRegular) {
    auto type = entriesCache->getEntryType("appimagetool.svg");
    ASSERT_EQ(type, appimage::core::PayloadEntryType::REGULAR);
}

TEST_F(TestPayloadEntriesCache, getEntryTypeLink) {
    auto type = entriesCache->getEntryType(".DirIcon");
    ASSERT_EQ(type, appimage::core::PayloadEntryType::LINK);
}

TEST_F(TestPayloadEntriesCache, getEntryTypeDir) {
    auto type = entriesCache->getEntryType("usr");
    ASSERT_EQ(type, appimage::core::PayloadEntryType::DIR);
}

TEST_F(TestPayloadEntriesCache, getLinkTarget1) {
    auto target = entriesCache->getEntryLinkTarget(".DirIcon");
    ASSERT_EQ(target, "appimagetool.svg");
}

TEST_F(TestPayloadEntriesCache, getLinkTargetNotLink) {
    ASSERT_THROW(entriesCache->getEntryLinkTarget("echo.destkop"), appimage::core::PayloadIteratorError);
}

TEST_F(TestPayloadEntriesCache, getMissingLinkTarget) {
    ASSERT_THROW(entriesCache->getEntryLinkTarget("missing"), appimage::core::PayloadIteratorError);
}
