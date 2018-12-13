// library headers
#include <gtest/gtest.h>

#include "core/integrator.h"
#include "utils/filesystem.h"
#include "file_management_utils.hpp"

using namespace appimage::desktop_integration::core;

class DesktopIntegrationTests : public ::testing::Test {
protected:
    std::string appdir_path;
    std::string user_dir_path;
    std::string appimage_path;

    virtual void SetUp() {
        appimage_path = std::string(TEST_DATA_DIR) + "Echo-x86_64.AppImage";

        appdir_path = createTempDir("libappimage-di-appdir");
        user_dir_path = createTempDir("libappimage-di-user-dir");

        ASSERT_FALSE(appdir_path.empty());
        ASSERT_FALSE(user_dir_path.empty());
    }

    virtual void TearDown() {
        removeDirRecursively(appdir_path);
        removeDirRecursively(user_dir_path);
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
            g_info("Coping %s to %s", source.c_str(), target.c_str());
            copy_file(source.c_str(), target.c_str());
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
