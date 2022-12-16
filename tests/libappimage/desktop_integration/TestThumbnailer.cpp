// system
#include <filesystem>
#include <fstream>
#include <sstream>

// library headers
#include <gtest/gtest.h>

// local
#include "appimage/desktop_integration/exceptions.h"
#include "Thumbnailer.h"
#include "utils/path_utils.h"
#include "TemporaryDirectory.h"

using namespace appimage::desktop_integration;

class TestThumbnailer : public ::testing::Test {
protected:
    const TemporaryDirectory xdgCacheHome{"xdg-cache-home"};

    void createStubFile(const std::filesystem::path& path, const std::string& content = "") {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream f(path);
        f << content;
    }
};

TEST_F(TestThumbnailer, createType1) {
    const std::string appImagePath = TEST_DATA_DIR "AppImageExtract_6-x86_64.AppImage";
    const Thumbnailer thumbnailer(xdgCacheHome.path());

    const appimage::core::AppImage appImage{appImagePath};
    thumbnailer.create(appImage);

    const auto canonicalAppImagePath = std::filesystem::weakly_canonical(appImagePath).string();
    const auto canonicalPathMd5 = appimage::utils::hashPath(canonicalAppImagePath);

    const auto normalIconPath = xdgCacheHome.path() / "thumbnails/normal" / (canonicalPathMd5 + ".png");
    const auto largeIconPath = xdgCacheHome.path() / "thumbnails/large" / (canonicalPathMd5 + ".png");

    ASSERT_TRUE(std::filesystem::exists(normalIconPath));
    ASSERT_FALSE(std::filesystem::is_empty(normalIconPath));

    ASSERT_TRUE(std::filesystem::exists(largeIconPath));
    ASSERT_FALSE(std::filesystem::is_empty(largeIconPath));
}


TEST_F(TestThumbnailer, createType2) {
    std::string appImagePath = TEST_DATA_DIR "Echo-x86_64.AppImage";
    Thumbnailer thumbnailer(xdgCacheHome.path());

    appimage::core::AppImage appImage{appImagePath};
    thumbnailer.create(appImage);

    std::string canonicalPathMd5 = appimage::utils::hashPath(appImagePath);

    const auto normalIconPath = xdgCacheHome.path() / "thumbnails/normal" / (canonicalPathMd5 + ".png");
    const auto largeIconPath = xdgCacheHome.path() / "thumbnails/large" / (canonicalPathMd5 + ".png");

    ASSERT_TRUE(std::filesystem::exists(normalIconPath));
    ASSERT_FALSE(std::filesystem::is_empty(normalIconPath));

    ASSERT_TRUE(std::filesystem::exists(largeIconPath));
    ASSERT_FALSE(std::filesystem::is_empty(largeIconPath));
}

TEST_F(TestThumbnailer, remove) {
    std::string appImagePath = TEST_DATA_DIR "Echo-x86_64.AppImage";
    Thumbnailer thumbnailer(xdgCacheHome.path());

    std::string canonicalPathMd5 = appimage::utils::hashPath(appImagePath);

    const auto normalIconPath = xdgCacheHome.path() / "thumbnails/normal" / (canonicalPathMd5 + ".png");
    const auto largeIconPath = xdgCacheHome.path() / "thumbnails/large" / (canonicalPathMd5 + ".png");


    createStubFile(normalIconPath);
    createStubFile(largeIconPath);

    ASSERT_TRUE(std::filesystem::exists(normalIconPath));
    ASSERT_TRUE(std::filesystem::exists(largeIconPath));

    thumbnailer.remove(appImagePath);

    ASSERT_FALSE(std::filesystem::exists(normalIconPath));
    ASSERT_FALSE(std::filesystem::exists(largeIconPath));
}
