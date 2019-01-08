// system
#include <sstream>

// library headers
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include "integrator/ResourcesExtractor.h"

using namespace appimage::desktop_integration::integrator;
namespace bf = boost::filesystem;

TEST(TestResourcesExtractor, extractDesktopIntegrartionResources) {
    ResourcesExtractor extractor(TEST_DATA_DIR "appimagetool-x86_64.AppImage");

    extractor.setExtractDesktopFile(true);
    extractor.setExtractIconFiles(true);
    extractor.setExtractAppDataFile(true);
    extractor.setExtractMimeFiles(false);

    auto resources = extractor.extract();
    ASSERT_FALSE(resources.desktopEntryPath.empty());
    ASSERT_FALSE(resources.desktopEntryData.empty());

    std::vector<std::string> expectedIcons = {".DirIcon"};

    for (auto expectedIconsItr = expectedIcons.begin();
         expectedIconsItr != expectedIcons.end(); ++expectedIconsItr)
        ASSERT_TRUE(resources.icons.find(*expectedIconsItr) != resources.icons.end());


    ASSERT_FALSE(resources.appStreamPath.empty());
    ASSERT_FALSE(resources.appStreamData.empty());
}
