// system
#include <sstream>

// library headers
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

// local
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

};

TEST_F(TestIntegrationManager, registerAppImage) {
    std::string appImagePath = TEST_DATA_DIR "Echo-x86_64.AppImage";
    IntegrationManager manager(userDirPath.string());
    manager.registerAppImage(appImagePath);

    const auto md5Digest = appimage::utils::HashLib::md5(appImagePath);
    std::string md5 = appimage::utils::HashLib::toHex(md5Digest);

    bf::path expectedDesktopFilePath = userDirPath / ("applications/appimagekit_" + md5 + "-Echo.desktop");
    ASSERT_TRUE(bf::exists(expectedDesktopFilePath));

    bf::path expectedIconFilePath =
        userDirPath / ("icons/hicolor/scalable/apps/appimagekit_" + md5 + "_utilities-terminal.svg");
    ASSERT_TRUE(bf::exists(expectedIconFilePath));
}
