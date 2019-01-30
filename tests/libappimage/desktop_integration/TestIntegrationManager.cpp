// system
#include <sstream>

// library headers
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

// local
#include "appimage/appimage.h"
#include "appimage/desktop_integration/Exceptions.h"
#include "appimage/desktop_integration/IntegrationManager.h"
#include "utils/HashLib.h"

using namespace appimage::desktop_integration;
namespace bf = boost::filesystem;

class TestIntegrationManager : public ::testing::Test {
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

    void createStubFile(const bf::path& path, const std::string& content = "") {
        bf::create_directories(path.parent_path());
        bf::ofstream f(path);
        f << content;
    }
};

TEST_F(TestIntegrationManager, registerAppImage) {
    std::string appImagePath = TEST_DATA_DIR "Echo-x86_64.AppImage";
    IntegrationManager manager(userDirPath.string());
    appimage::core::AppImage appImage(appImagePath);
    manager.registerAppImage(appImage);

    std::string md5 = appimage_get_md5(appImagePath.c_str()) ?: "";

    bf::path expectedDesktopFilePath = userDirPath / ("applications/appimagekit_" + md5 + "-Echo.desktop");
    ASSERT_TRUE(bf::exists(expectedDesktopFilePath));

    bf::path expectedIconFilePath =
        userDirPath / ("icons/hicolor/scalable/apps/appimagekit_" + md5 + "_utilities-terminal.svg");
    ASSERT_TRUE(bf::exists(expectedIconFilePath));
}

TEST_F(TestIntegrationManager, isARegisteredAppImage) {
    std::string appImagePath = TEST_DATA_DIR "Echo-x86_64.AppImage";
    IntegrationManager manager(userDirPath.string());

    appimage::core::AppImage appImage(appImagePath);
    ASSERT_FALSE(manager.isARegisteredAppImage(appImage));

    { // Generate fake desktop entry file
        std::string md5 = appimage_get_md5(appImagePath.c_str()) ?: "";

        bf::path desployedDesktopFilePath = userDirPath / ("applications/appimagekit_" + md5 + "-Echo.desktop");
        createStubFile(desployedDesktopFilePath, "[Desktop Entry]");

        ASSERT_TRUE(bf::exists(desployedDesktopFilePath));
    }

    ASSERT_TRUE(manager.isARegisteredAppImage(appImage));
}

TEST_F(TestIntegrationManager, shallAppImageBeRegistered) {
    IntegrationManager manager;

    ASSERT_TRUE(manager.shallAppImageBeRegistered(
        appimage::core::AppImage(TEST_DATA_DIR "Echo-x86_64.AppImage")));
    ASSERT_FALSE(manager.shallAppImageBeRegistered(
        appimage::core::AppImage(TEST_DATA_DIR "Echo-no-integrate-x86_64.AppImage")));
    ASSERT_THROW(manager.shallAppImageBeRegistered(
        appimage::core::AppImage(TEST_DATA_DIR "elffile")), appimage::core::AppImageError);
}


TEST_F(TestIntegrationManager, unregisterAppImage) {
    std::string appImagePath = TEST_DATA_DIR "Echo-x86_64.AppImage";
    IntegrationManager manager(userDirPath.string());

    // Generate fake desktop entry file
    std::string md5 = appimage_get_md5(appImagePath.c_str()) ?: "";

    bf::path desployedDesktopFilePath = userDirPath / ("applications/appimagekit_" + md5 + "-Echo.desktop");
    createStubFile(desployedDesktopFilePath, "[Desktop Entry]");
    ASSERT_TRUE(bf::exists(desployedDesktopFilePath));

    bf::path desployedIconFilePath = userDirPath /
                                     ("icons/hicolor/scalable/apps/appimagekit_" + md5 + "_utilities-terminal.svg");
    createStubFile(desployedIconFilePath, "<?xml");
    ASSERT_TRUE(bf::exists(desployedIconFilePath));


    bf::path desployedMimeTypePackageFilePath = userDirPath / ("mime/packages/appimagekit_" + md5 + "-echo.xml");
    createStubFile(desployedMimeTypePackageFilePath, "<?xml");
    ASSERT_TRUE(bf::exists(desployedMimeTypePackageFilePath));

    appimage::core::AppImage appImage(appImagePath);
    manager.unregisterAppImage(appImage);

    ASSERT_FALSE(bf::exists(desployedDesktopFilePath));
    ASSERT_FALSE(bf::exists(desployedIconFilePath));
    ASSERT_FALSE(bf::exists(desployedMimeTypePackageFilePath));
}
