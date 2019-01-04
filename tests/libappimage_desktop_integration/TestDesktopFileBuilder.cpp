//system
#include <sstream>

// library
#include <gtest/gtest.h>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include <appimage/core/Exceptions.h>
#include "utils/HashLib.h"
#include "DesktopIntegrationErrors.h"
#include "DesktopEntryBuilder.h"

using namespace appimage::desktop_integration;
using namespace appimage::utils;

class DesktopFileBuilderTests : public ::testing::Test {
protected:
    DesktopEntryBuilder builder;
    std::stringstream originalData;
protected:

    void SetUp() override {
        originalData << "[Desktop Entry]\n"
                     << "Version=1.0\n"
                     << "Type=Application\n"
                     << "Name=Foo Viewer\n"
                     << "Comment=The best viewer for Foo objects available!\n"
                     << "TryExec=fooview\n"
                     << "Exec=fooview %F\n"
                     << "Icon=fooview\n"
                     << "Icon[es]=fooview-es\n"
                     << "MimeType=image/x-foo;\n"
                     << "Actions=Gallery;Create;\n"
                     << "\n"
                     << "[Desktop Action Gallery]\n"
                     << "Exec=fooview --gallery\n"
                     << "Name=Browse Gallery\n"
                     << "\n"
                     << "[Desktop Action Create]\n"
                     << "Exec=fooview --create-new\n"
                     << "Name=Create a new Foo!\n"
                     << "Icon=fooview-new";
    }
};

TEST_F(DesktopFileBuilderTests, setPath) {
    std::string path = TEST_DATA_DIR "Echo-x86_64.AppImage";
    builder.setAppImagePath(path);

    ASSERT_THROW(builder.build(), DesktopEntryBuildError);

    builder.setUuid("uuid");
    builder.setBaseDesktopFile(originalData);

    XdgUtils::DesktopEntry::DesktopEntry result(builder.build());

    ASSERT_EQ(result.get("Desktop Entry/Exec"), path + " %F");
    ASSERT_EQ(result.get("Desktop Entry/TryExec"), path);
    ASSERT_EQ(result.get("Desktop Action Gallery/Exec"), path + " --gallery");
    ASSERT_EQ(result.get("Desktop Action Create/Exec"), path + " --create-new");
}

TEST_F(DesktopFileBuilderTests, setIcons) {
    std::string path = TEST_DATA_DIR "Echo-x86_64.AppImage";

    builder.setBaseDesktopFile(originalData);
    builder.setVendorPrefix("test");

    std::string appImagePathMd5 = HashLib::toHex(HashLib::md5(path));
    builder.setUuid(appImagePathMd5);

    XdgUtils::DesktopEntry::DesktopEntry result(builder.build());

    ASSERT_EQ(result.get("Desktop Entry/Icon"), "test_" + appImagePathMd5 + "_fooview");
    ASSERT_EQ(result.get("Desktop Entry/Icon[es]"), "test_" + appImagePathMd5 + "_fooview-es");
    ASSERT_EQ(result.get("Desktop Action Create/Icon"), "test_" + appImagePathMd5 + "_fooview-new");

    ASSERT_EQ(result.get("Desktop Entry/X-AppImage-Old-Icon"), "fooview");
    ASSERT_EQ(result.get("Desktop Entry/X-AppImage-Old-Icon[es]"), "fooview-es");
    ASSERT_EQ(result.get("Desktop Action Create/X-AppImage-Old-Icon"), "fooview-new");
}
