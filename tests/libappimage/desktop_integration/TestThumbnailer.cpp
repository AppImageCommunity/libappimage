// system
#include <sstream>

// library headers
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

// local
#include "appimage/appimage.h"
#include "appimage/desktop_integration/Exceptions.h"
#include "Thumbnailer.h"

using namespace appimage::desktop_integration;
namespace bf = boost::filesystem;

class TestThumbnailer : public ::testing::Test {
protected:
    bf::path xdgCacheHome;

    void SetUp() override {
        xdgCacheHome = bf::temp_directory_path() / boost::filesystem::unique_path();
        bf::create_directories(xdgCacheHome);

        ASSERT_FALSE(xdgCacheHome.empty());
    }

    void TearDown() override {
        bf::remove_all(xdgCacheHome);
    }

    void createStubFile(const bf::path& path, const std::string& content = "") {
        bf::create_directories(path.parent_path());
        bf::ofstream f(path);
        f << content;
    }
};

TEST_F(TestThumbnailer, create) {
    std::string appImagePath = TEST_DATA_DIR "Echo-x86_64.AppImage";
    Thumbnailer thumbnailer(appImagePath, xdgCacheHome.string());

    thumbnailer.create();

    std::string cannonicalPath = "file://" + appImagePath;
    std::string md5Sum = appimage_get_md5(cannonicalPath.c_str()) ?: "";
    bf::path normalIconPath = xdgCacheHome / "thumbnails/normal" / (md5Sum + ".png");
    bf::path largeIconPath = xdgCacheHome / "thumbnails/large" / (md5Sum + ".png");

    ASSERT_TRUE(bf::exists(normalIconPath));
    ASSERT_TRUE(bf::exists(largeIconPath));
}
