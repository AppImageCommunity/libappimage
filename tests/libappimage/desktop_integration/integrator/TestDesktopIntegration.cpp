// system
#include <sstream>
#include <iostream>
#include <fstream>

// library headers
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include "appimage/desktop_integration/exceptions.h"
#include "integrator/Integrator.h"
#include "integrator/MimeInfoEditor.h"
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
    Integrator i(appImage, userDirPath.string());

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
    Integrator i(appImage, userDirPath.string());

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
    Integrator i(appImage, userDirPath.string());

    ASSERT_THROW(i.integrate(), appimage::desktop_integration::DesktopIntegrationError);
}

TEST_F(DesktopIntegrationTests, emtpyXdgDataDir) {
    appimage::core::AppImage appImage(TEST_DATA_DIR "Echo-no-integrate-x86_64.AppImage");

    ASSERT_THROW(Integrator(appImage, ""), appimage::desktop_integration::DesktopIntegrationError);
}

TEST_F(DesktopIntegrationTests, malformedDesktopEntry) {
    appimage::core::AppImage appImage(TEST_DATA_DIR "broken-desktop-file-x86_64.AppImage");

    ASSERT_THROW(Integrator(appImage, userDirPath.string()), appimage::desktop_integration::DesktopIntegrationError);
}

TEST_F(DesktopIntegrationTests, integrateMimeType) {
    std::string appImagePath = NEW_TEST_DATA_DIR "echo.with.mimetype.AppImage";
    appimage::core::AppImage appImage(appImagePath);
    Integrator i(appImage, userDirPath.string());

    i.integrate();

    std::string md5 = appimage::utils::hashPath(appImagePath.c_str());

    bf::path mimeTypeIconFilePath =
        userDirPath / ("icons/hicolor/scalable/mimetypes/appimagekit_" + md5 + "_application-x-custom-file.svg");
    ASSERT_TRUE(bf::exists(mimeTypeIconFilePath));

    bf::path mimeTypePackageFilePath = userDirPath / ("mime/packages/appimagekit_" + md5 + "_custom.xml");
    ASSERT_TRUE(bf::exists(mimeTypePackageFilePath));

    // Read generated mime info package file
    std::ifstream mimeTypePackageFile(mimeTypePackageFilePath.string());
    std::string fileData((std::istreambuf_iterator<char>(mimeTypePackageFile)),
                         std::istreambuf_iterator<char>());

    MimeInfoEditor editor(fileData);
    // Compare icon names
    auto result = editor.getMimeTypeIconNames();
    std::list<std::string> expected = {"appimagekit_" + md5 + "_application-x-custom-file"};
    ASSERT_EQ(result, expected);
}

