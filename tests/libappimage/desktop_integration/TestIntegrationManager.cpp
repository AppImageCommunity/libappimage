// system
#include <sstream>

// library headers
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

// local
#include "appimage/desktop_integration/exceptions.h"
#include "appimage/desktop_integration/IntegrationManager.h"
#include "utils/hashlib.h"
#include "utils/path_utils.h"
#include "TemporaryDirectory.h"

using namespace appimage::desktop_integration;

class TestIntegrationManager : public ::testing::Test {
protected:
    const TemporaryDirectory userDir{"user-dir"};

    void createStubFile(const std::filesystem::path& path, const std::string& content = "") {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream f(path);
        f << content;
    }
};

TEST_F(TestIntegrationManager, registerAppImage) {
    const std::string appImagePath = TEST_DATA_DIR "Echo-x86_64.AppImage";

    const IntegrationManager manager(userDir.path());

    const appimage::core::AppImage appImage(appImagePath);
    manager.registerAppImage(appImage);

    const auto md5 = appimage::utils::hashPath(appImagePath.c_str());

    const auto expectedDesktopFilePath = userDir.path() / ("applications/appimagekit_" + md5 + "-Echo.desktop");
    ASSERT_TRUE(std::filesystem::exists(expectedDesktopFilePath));

    auto expectedIconFilePath = userDir.path() / ("icons/hicolor/scalable/apps/appimagekit_" + md5 + "_utilities-terminal.svg");
    ASSERT_TRUE(std::filesystem::exists(expectedIconFilePath));
}

TEST_F(TestIntegrationManager, isARegisteredAppImage) {
    const std::string appImagePath = TEST_DATA_DIR "Echo-x86_64.AppImage";
    const IntegrationManager manager(userDir.path());

    ASSERT_FALSE(manager.isARegisteredAppImage(appImagePath));

    {
        // Generate fake desktop entry file
        const auto md5 = appimage::utils::hashPath(appImagePath.c_str());

        const auto desployedDesktopFilePath = userDir.path() / ("applications/appimagekit_" + md5 + "-Echo.desktop");
        createStubFile(desployedDesktopFilePath, "[Desktop Entry]");

        ASSERT_TRUE(std::filesystem::exists(desployedDesktopFilePath));
    }

    ASSERT_TRUE(manager.isARegisteredAppImage(appImagePath));
}

TEST_F(TestIntegrationManager, shallAppImageBeRegistered) {
    const IntegrationManager manager;

    ASSERT_TRUE(manager.shallAppImageBeRegistered(
        appimage::core::AppImage(TEST_DATA_DIR "Echo-x86_64.AppImage")));
    ASSERT_FALSE(manager.shallAppImageBeRegistered(
        appimage::core::AppImage(TEST_DATA_DIR "Echo-no-integrate-x86_64.AppImage")));
    ASSERT_THROW(manager.shallAppImageBeRegistered(
        appimage::core::AppImage(TEST_DATA_DIR "elffile")), appimage::core::AppImageError);
}


TEST_F(TestIntegrationManager, unregisterAppImage) {
    const std::string appImagePath = TEST_DATA_DIR "Echo-x86_64.AppImage";
    const IntegrationManager manager(userDir.path());

    // Generate fake desktop entry file
    const auto md5 = appimage::utils::hashPath(appImagePath.c_str());

    const auto deployedDesktopFilePath = userDir.path() / ("applications/appimagekit_" + md5 + "-Echo.desktop");
    createStubFile(deployedDesktopFilePath, "[Desktop Entry]");
    ASSERT_TRUE(std::filesystem::exists(deployedDesktopFilePath));

    const auto desployedIconFilePath = userDir.path() / ("icons/hicolor/scalable/apps/appimagekit_" + md5 + "_utilities-terminal.svg");
    createStubFile(desployedIconFilePath, "<?xml");
    ASSERT_TRUE(std::filesystem::exists(desployedIconFilePath));


    const auto deployedMimeTypePackageFilePath = userDir.path() / ("mime/packages/appimagekit_" + md5 + "-echo.xml");
    createStubFile(deployedMimeTypePackageFilePath, "<?xml");
    ASSERT_TRUE(std::filesystem::exists(deployedMimeTypePackageFilePath));

    manager.unregisterAppImage(appImagePath);

    ASSERT_FALSE(std::filesystem::exists(deployedDesktopFilePath));
    ASSERT_FALSE(std::filesystem::exists(desployedIconFilePath));
    ASSERT_FALSE(std::filesystem::exists(deployedMimeTypePackageFilePath));
}
