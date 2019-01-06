// system
#include <sstream>

// library headers
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include "ResourcesExtractor.h"

using namespace appimage::desktop_integration;
namespace bf = boost::filesystem;

TEST(TestResourcesExtractor, extractDesktopIntegrartionResources) {
    ResourcesExtractor extractor(TEST_DATA_DIR "appimagetool-x86_64.AppImage");

    extractor.setExtractDesktopFile(true);
    extractor.setExtractIconFiles(true);
    extractor.setExtractAppDataFile(true);
    extractor.setExtractMimeFiles(false);

    auto resources = extractor.extract();
    ASSERT_TRUE(resources.find("appimagetool.desktop") != resources.end());

    std::vector<std::string> expectedIcons = {".DirIcon"};

    for (auto expectedIconsItr = expectedIcons.begin();
         expectedIconsItr != expectedIcons.end(); ++expectedIconsItr)
        ASSERT_TRUE(resources.find(*expectedIconsItr) != resources.end());


    ASSERT_TRUE(resources.find("usr/share/metainfo/appimagetool.appdata.xml") != resources.end());
}
