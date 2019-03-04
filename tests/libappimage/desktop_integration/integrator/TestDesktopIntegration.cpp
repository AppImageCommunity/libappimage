// system
#include <sstream>

// library headers
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include "appimage/desktop_integration/exceptions.h"
#include "integrator/Integrator.h"
#include "utils/hashlib.h"
#include "utils/path_utils.h"

using namespace appimage::desktop_integration::integrator;
namespace bf = boost::filesystem;

class DesktopIntegrationTests : public ::testing::Test {
protected:
    bf::path userDirPath;

    void SetUp() override {
        userDirPath = bf::temp_directory_path() / boost::filesystem::unique_path();
        bf::create_directories(userDirPath);

        ASSERT_FALSE(userDirPath.empty());
    }

    void TearDown() override {
        bf::remove_all(userDirPath);
    }

};

TEST_F(DesktopIntegrationTests, integrateEchoAppImage) {
    std::string appImagePath = TEST_DATA_DIR "Echo-x86_64.AppImage";
    appimage::core::AppImage appImage(appImagePath);
    Integrator i(appImage, userDirPath.string(), "appimagekit");

    i.integrate();

    std::string md5 = appimage::utils::hashPath(appImagePath.c_str());

    bf::path expectedDesktopFilePath = userDirPath / ("applications/appimagekit_" + md5 + "-Echo.desktop");
    ASSERT_TRUE(bf::exists(expectedDesktopFilePath));

    bf::path expectedIconFilePath =
        userDirPath / ("icons/hicolor/scalable/apps/appimagekit_" + md5 + "_utilities-terminal.svg");
    ASSERT_TRUE(bf::exists(expectedIconFilePath));
}

TEST_F(DesktopIntegrationTests, integrateAppImageExtract) {
    std::string appImagePath = TEST_DATA_DIR "AppImageExtract_6-x86_64.AppImage";
    appimage::core::AppImage appImage(appImagePath);
    Integrator i(appImage, userDirPath.string(), "appimagekit");

    i.integrate();

    std::string md5 = appimage::utils::hashPath(appImagePath.c_str());

    bf::path expectedDesktopFilePath = userDirPath / ("applications/appimagekit_" + md5 + "-AppImageExtract.desktop");
    ASSERT_TRUE(bf::exists(expectedDesktopFilePath));

    bf::path expectedIconFilePath =
        userDirPath / ("icons/hicolor/48x48/apps/appimagekit_" + md5 + "_AppImageExtract.png");
    ASSERT_TRUE(bf::exists(expectedIconFilePath));
}

TEST_F(DesktopIntegrationTests, integrateEchoNoIntegrate) {
    appimage::core::AppImage appImage(TEST_DATA_DIR "Echo-no-integrate-x86_64.AppImage");
    Integrator i(appImage, userDirPath.string(), "appimagekit");

    ASSERT_THROW(i.integrate(), appimage::desktop_integration::DesktopIntegrationError);
}
