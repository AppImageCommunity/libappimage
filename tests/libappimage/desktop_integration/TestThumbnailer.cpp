// system
#include <sstream>

// library headers
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

// local
#include "appimage/desktop_integration/exceptions.h"
#include "Thumbnailer.h"
#include "utils/path_utils.h"

using namespace appimage::desktop_integration;
namespace bf = boost::filesystem;

class TestThumbnailer : public ::testing::Test {
protected:
    bf::path xdgCacheHome = "";

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

TEST_F(TestThumbnailer, createType1) {
    std::string appImagePath = TEST_DATA_DIR "AppImageExtract_6-x86_64.AppImage";
    Thumbnailer thumbnailer(xdgCacheHome.string());

    appimage::core::AppImage appImage{appImagePath};
    thumbnailer.create(appImage);

    auto canonicalAppImagePath = boost::filesystem::weakly_canonical(appImagePath).string();
    std::string canonicalPathMd5 = appimage::utils::hashPath(canonicalAppImagePath);

    bf::path normalIconPath = xdgCacheHome / "thumbnails/normal" / (canonicalPathMd5 + ".png");
    bf::path largeIconPath = xdgCacheHome / "thumbnails/large" / (canonicalPathMd5 + ".png");

    ASSERT_TRUE(bf::exists(normalIconPath));
    ASSERT_FALSE(bf::is_empty(normalIconPath));

    ASSERT_TRUE(bf::exists(largeIconPath));
    ASSERT_FALSE(bf::is_empty(largeIconPath));
}


TEST_F(TestThumbnailer, createType2) {
    std::string appImagePath = TEST_DATA_DIR "Echo-x86_64.AppImage";
    Thumbnailer thumbnailer(xdgCacheHome.string());

    appimage::core::AppImage appImage{appImagePath};
    thumbnailer.create(appImage);

    std::string canonicalPathMd5 = appimage::utils::hashPath(appImagePath);

    bf::path normalIconPath = xdgCacheHome / "thumbnails/normal" / (canonicalPathMd5 + ".png");
    bf::path largeIconPath = xdgCacheHome / "thumbnails/large" / (canonicalPathMd5 + ".png");

    ASSERT_TRUE(bf::exists(normalIconPath));
    ASSERT_FALSE(bf::is_empty(normalIconPath));

    ASSERT_TRUE(bf::exists(largeIconPath));
    ASSERT_FALSE(bf::is_empty(largeIconPath));
}

TEST_F(TestThumbnailer, remove) {
    std::string appImagePath = TEST_DATA_DIR "Echo-x86_64.AppImage";
    Thumbnailer thumbnailer(xdgCacheHome.string());

    std::string canonicalPathMd5 = appimage::utils::hashPath(appImagePath);

    bf::path normalIconPath = xdgCacheHome / "thumbnails/normal" / (canonicalPathMd5 + ".png");
    bf::path largeIconPath = xdgCacheHome / "thumbnails/large" / (canonicalPathMd5 + ".png");


    createStubFile(normalIconPath);
    createStubFile(largeIconPath);

    ASSERT_TRUE(bf::exists(normalIconPath));
    ASSERT_TRUE(bf::exists(largeIconPath));

    appimage::core::AppImage appImage{appImagePath};
    thumbnailer.remove(appImage);

    ASSERT_FALSE(bf::exists(normalIconPath));
    ASSERT_FALSE(bf::exists(largeIconPath));
}
