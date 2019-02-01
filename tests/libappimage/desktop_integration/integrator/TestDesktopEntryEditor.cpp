//system
#include <sstream>

// library
#include <gtest/gtest.h>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include <appimage/core/exceptions.h>
#include "utils/HashLib.h"
#include "integrator/DesktopEntryEditError.h"
#include "integrator/DesktopEntryEditor.h"

using namespace appimage::desktop_integration::integrator;
using namespace appimage::utils;

class DesktopEntryEditorTests : public ::testing::Test {
protected:
    std::stringstream originalData;
protected:

    void SetUp() override {
        originalData << "[Desktop Entry]\n"
                     << "Version=1.0\n"
                     << "Type=Application\n"
                     << "Name=Foo Viewer\n"
                     << "Name[es]=Visor de Foo\n"
                     << "Name[en]=Foo Viewer 0.1.1\n"
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

TEST_F(DesktopEntryEditorTests, setPath) {
    XdgUtils::DesktopEntry::DesktopEntry entry(originalData);
    DesktopEntryEditor editor;
    std::string path = TEST_DATA_DIR "Echo-x86_64.AppImage";
    editor.setAppImagePath(path);
    editor.setIdentifier("uuid");
    editor.edit(entry);

    ASSERT_EQ(entry.get("Desktop Entry/Exec"), path + " %F");
    ASSERT_EQ(entry.get("Desktop Entry/TryExec"), path);
    ASSERT_EQ(entry.get("Desktop Action Gallery/Exec"), path + " --gallery");
    ASSERT_EQ(entry.get("Desktop Action Create/Exec"), path + " --create-new");
}

TEST_F(DesktopEntryEditorTests, setIcons) {
    XdgUtils::DesktopEntry::DesktopEntry entry(originalData);
    DesktopEntryEditor editor;
    std::string path = TEST_DATA_DIR "Echo-x86_64.AppImage";

    editor.setVendorPrefix("test");

    std::string appImagePathMd5 = HashLib::toHex(HashLib::md5(path));
    editor.setIdentifier(appImagePathMd5);
    editor.edit(entry);

    ASSERT_EQ(entry.get("Desktop Entry/Icon"), "test_" + appImagePathMd5 + "_fooview");
    ASSERT_EQ(entry.get("Desktop Entry/Icon[es]"), "test_" + appImagePathMd5 + "_fooview-es");
    ASSERT_EQ(entry.get("Desktop Action Create/Icon"), "test_" + appImagePathMd5 + "_fooview-new");

    ASSERT_EQ(entry.get("Desktop Entry/X-AppImage-Old-Icon"), "fooview");
    ASSERT_EQ(entry.get("Desktop Entry/X-AppImage-Old-Icon[es]"), "fooview-es");
    ASSERT_EQ(entry.get("Desktop Action Create/X-AppImage-Old-Icon"), "fooview-new");
}

TEST_F(DesktopEntryEditorTests, setVersion) {
    XdgUtils::DesktopEntry::DesktopEntry entry(originalData);
    DesktopEntryEditor editor;
    std::string path = TEST_DATA_DIR "Echo-x86_64.AppImage";

    editor.setVendorPrefix("prefix");
    editor.setIdentifier("uuid");

    editor.setAppImageVersion("0.1.1");
    editor.edit(entry);

    ASSERT_EQ(entry.get("Desktop Entry/Name"), "Foo Viewer (0.1.1)");
    ASSERT_EQ(entry.get("Desktop Entry/Name[en]"), "Foo Viewer 0.1.1");
    ASSERT_EQ(entry.get("Desktop Entry/Name[es]"), "Visor de Foo (0.1.1)");

    ASSERT_EQ(entry.get("Desktop Entry/X-AppImage-Old-Name"), "Foo Viewer");
    ASSERT_FALSE(entry.exists("Desktop Entry/X-AppImage-Old-Name[en]"));
    ASSERT_EQ(entry.get("Desktop Entry/X-AppImage-Old-Name[es]"), "Visor de Foo");
}

TEST_F(DesktopEntryEditorTests, setIdentifier) {
    XdgUtils::DesktopEntry::DesktopEntry entry(originalData);

    DesktopEntryEditor editor;
    editor.setVendorPrefix("prefix");
    editor.setIdentifier("uuid");
    editor.setAppImageVersion("0.1.1");
    editor.edit(entry);

    ASSERT_EQ(entry.get("Desktop Entry/X-AppImage-Identifier"), "uuid");
}
