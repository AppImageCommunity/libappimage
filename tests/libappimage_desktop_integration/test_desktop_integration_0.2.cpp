// system
#include <sstream>

// library headers
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include "appimage/desktop_integration/Integrator.h"
#include "utils/HashLib.h"

using namespace appimage::desktop_integration;
namespace bf = boost::filesystem;

class DesktopIntegrationTests : public ::testing::Test {
protected:
    bf::path appDirPath;
    bf::path userDirPath;
    bf::path appimagePath;

    void SetUp() override {
        appimagePath = std::string(TEST_DATA_DIR) + "Echo-x86_64.AppImage";

        appDirPath = bf::temp_directory_path() / boost::filesystem::unique_path();
        userDirPath = bf::temp_directory_path() / boost::filesystem::unique_path();

        bf::create_directories(appDirPath);
        bf::create_directories(userDirPath);

        ASSERT_FALSE(appDirPath.empty());
        ASSERT_FALSE(userDirPath.empty());
    }

    void TearDown() override {
        bf::remove_all(appDirPath);
        bf::remove_all(userDirPath);
    }

    void fillMinimalAppDir() {
        std::map<std::string, std::string> files;
        files["squashfs-root/usr/bin/echo"] = "usr/bin/echo";
        files["squashfs-root/utilities-terminal.svg"] = ".DirIcon";
        files["squashfs-root/AppRun"] = "AppRun";
        files["squashfs-root/echo.desktop"] = "echo.desktop";

        copy_files(files);
    }

    void copy_files(std::map<std::string, std::string>& files) const {
        for (auto itr = files.begin(); itr != files.end(); itr++) {
            bf::path source = std::string(TEST_DATA_DIR) + "/" + itr->first;
            bf::path target = appDirPath / itr->second;

            std::clog << "Coping " << source << " to " << target << std::endl;
            bf::copy_file(source, target, bf::copy_option::overwrite_if_exists);
        }
    }
};

TEST_F(DesktopIntegrationTests, getDesktopFilePath) {
    Integrator i(appimagePath.string(), userDirPath.string());

    // build expected value
    std::istringstream is(appimagePath.string());
    const auto md5Digest = appimage::utils::HashLib::md5(is);
    std::string md5 = appimage::utils::HashLib::toHex(md5Digest);
    std::string expectedDesktopFilePath = userDirPath.string() + "/applications/appimagekit_" + md5 + "-Echo.desktop";

    std::string desktopFilePath = i.getDesktopFilePath();
    ASSERT_EQ(desktopFilePath, expectedDesktopFilePath);
}
