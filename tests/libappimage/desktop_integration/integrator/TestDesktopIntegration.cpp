// system
#include <sstream>

// library headers
#include <gtest/gtest.h>
#include <filesystem>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include "appimage/desktop_integration/exceptions.h"
#include "integrator/Integrator.h"
#include "utils/hashlib.h"
#include "utils/path_utils.h"
#include "TemporaryDirectory.h"

using namespace appimage::desktop_integration::integrator;

class DesktopIntegrationTests : public ::testing::Test {
protected:
    const TemporaryDirectory userDir{"user-dir"};
};

TEST_F(DesktopIntegrationTests, integrateEchoAppImage) {
    const std::string appImagePath = TEST_DATA_DIR "Echo-x86_64.AppImage";
    const appimage::core::AppImage appImage(appImagePath);
    const Integrator i(appImage, userDir.path());

    i.integrate();

    std::string md5 = appimage::utils::hashPath(appImagePath.c_str());

    std::filesystem::path expectedDesktopFilePath = userDir.path() / ("applications/appimagekit_" + md5 + "-Echo.desktop");
    ASSERT_TRUE(std::filesystem::exists(expectedDesktopFilePath));

    std::filesystem::path expectedIconFilePath =
        userDir.path() / ("icons/hicolor/scalable/apps/appimagekit_" + md5 + "_utilities-terminal.svg");
    ASSERT_TRUE(std::filesystem::exists(expectedIconFilePath));
}

TEST_F(DesktopIntegrationTests, integrateAppImageExtract) {
    std::string appImagePath = TEST_DATA_DIR "AppImageExtract_6-x86_64.AppImage";
    appimage::core::AppImage appImage(appImagePath);
    Integrator i(appImage, userDir.path());

    i.integrate();

    std::string md5 = appimage::utils::hashPath(appImagePath.c_str());

    std::filesystem::path expectedDesktopFilePath = userDir.path() / ("applications/appimagekit_" + md5 + "-AppImageExtract.desktop");
    ASSERT_TRUE(std::filesystem::exists(expectedDesktopFilePath));

    auto expectedIconFilePath = userDir.path() / ("icons/hicolor/48x48/apps/appimagekit_" + md5 + "_AppImageExtract.png");
    ASSERT_TRUE(std::filesystem::exists(expectedIconFilePath));
}

TEST_F(DesktopIntegrationTests, integrateEchoNoIntegrate) {
    appimage::core::AppImage appImage(TEST_DATA_DIR "Echo-no-integrate-x86_64.AppImage");
    const Integrator i(appImage, userDir.path());

    ASSERT_THROW(i.integrate(), appimage::desktop_integration::DesktopIntegrationError);
}

TEST_F(DesktopIntegrationTests, emtpyXdgDataDir) {
    appimage::core::AppImage appImage(TEST_DATA_DIR "Echo-no-integrate-x86_64.AppImage");

    ASSERT_THROW(Integrator(appImage, ""), appimage::desktop_integration::DesktopIntegrationError);
}

TEST_F(DesktopIntegrationTests, malformedDesktopEntry) {
    appimage::core::AppImage appImage(TEST_DATA_DIR "broken-desktop-file-x86_64.AppImage");

    ASSERT_THROW(Integrator(appImage, userDir.path()), appimage::desktop_integration::DesktopIntegrationError);
}
