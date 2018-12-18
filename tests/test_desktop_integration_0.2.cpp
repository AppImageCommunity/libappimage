// library headers
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

#include "core/integrator.h"

using namespace appimage::desktop_integration::core;
namespace bf = boost::filesystem;

class DesktopIntegrationTests : public ::testing::Test {
protected:
    std::string appdir_path;
    std::string user_dir_path;
    std::string appimage_path;

    virtual void SetUp() {
        appimage_path = std::string(TEST_DATA_DIR) + "Echo-x86_64.AppImage";

        appdir_path = bf::create_directories(bf::temp_directory_path() / boost::filesystem::unique_path());
        user_dir_path = bf::create_directories(bf::temp_directory_path() / boost::filesystem::unique_path());

        ASSERT_FALSE(appdir_path.empty());
        ASSERT_FALSE(user_dir_path.empty());
    }

    virtual void TearDown() {
        bf::remove_all(appdir_path);
        bf::remove_all(user_dir_path);
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
            std::string source = std::string(TEST_DATA_DIR) + "/" + itr->first;
            std::string target = appdir_path + "/" + itr->second;

            std::clog << "Coping " << source << " to " << target << std::endl;
            bf::copy_file(source, target, bf::copy_option::overwrite_if_exists);
        }
    }
};

TEST_F(DesktopIntegrationTests, create) {
    integrator i(appimage_path);
}

TEST_F(DesktopIntegrationTests, integrate) {
    integrator i(appimage_path, user_dir_path);
    i.integrate();


}
