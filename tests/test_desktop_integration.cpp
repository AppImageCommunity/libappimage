// library headers
#include <glib.h>
#include <gtest/gtest.h>
#include <glib/gstdio.h>
#include <appimage/appimage.h>
#include <fcntl.h>
extern "C" {
#include <unistd.h>
}

#include "desktop_integration.h"
#include "file_management_utils.hpp"

class DesktopIntegrationTests : public ::testing::Test {
protected:
    std::string appdir_path;
    std::string user_dir_path;
    char* appimage_path;

    virtual void SetUp() {
        appimage_path = g_strjoin("/", TEST_DATA_DIR, "Echo-x86_64.AppImage", NULL);

        appdir_path = createTempDir("libappimage-di-appdir");
        user_dir_path = createTempDir("libappimage-di-user-dir");

        ASSERT_FALSE(appdir_path.empty());
        ASSERT_FALSE(user_dir_path.empty());
    }

    virtual void TearDown() {
        removeDirRecursively(appdir_path);
        removeDirRecursively(user_dir_path);

        g_free(appimage_path);
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
        for (std::map<std::string, std::string>::iterator itr = files.begin(); itr != files.end(); itr++) {
            std::string source = std::string(TEST_DATA_DIR) + "/" + itr->first;
            std::string target = appdir_path + "/" + itr->second;
            g_info("Coping %s to %s", source.c_str(), target.c_str());
            copy_file(source.c_str(), target.c_str());
        }
    }
};

TEST_F(DesktopIntegrationTests, create_tempdir) {
    char* tempdir = desktop_integration_create_tempdir();
    ASSERT_TRUE(g_file_test(tempdir, G_FILE_TEST_IS_DIR));
    ASSERT_TRUE(g_file_test(tempdir, G_FILE_TEST_EXISTS));

    g_rmdir(tempdir);

    ASSERT_FALSE(g_file_test(tempdir, G_FILE_TEST_IS_DIR));
    ASSERT_FALSE(g_file_test(tempdir, G_FILE_TEST_EXISTS));

    free(tempdir);
}

TEST_F(DesktopIntegrationTests, extract_relevant_files) {
    // Test body
    desktop_integration_extract_relevant_files(appimage_path, appdir_path.c_str());

    GDir* tempdir = NULL;
    tempdir = g_dir_open(appdir_path.c_str(), 0, NULL);
    if (!tempdir)
        FAIL();

    const char* entry;
    bool desktop_file_found = false;
    while ((entry = g_dir_read_name(tempdir)) != NULL) {
        if (g_str_has_suffix(entry, ".Desktop") || g_str_has_suffix(entry, ".desktop"))
            desktop_file_found = true;
    }

    g_dir_close(tempdir);
    ASSERT_TRUE(desktop_file_found);
}


extern "C" char* find_desktop_file(const char* path);
extern "C" GKeyFile* load_desktop_file(const char* file_path);

char* extract_exec_args_from_desktop(GKeyFile* original_desktop_file) {
    char* original_exec_value = g_key_file_get_string(original_desktop_file,
                                                      G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
    g_key_file_free(original_desktop_file);

    char** original_exec_value_parts = g_strsplit_set(original_exec_value, " ", 2);
    char* original_exec_value_args = NULL;
    char** ptr = original_exec_value_parts;
    if (*ptr != NULL)
        ptr++;
    if (*ptr != NULL)
        original_exec_value_args = g_strdup(*ptr);

    for (ptr = original_exec_value_parts; *ptr != NULL; ptr++)
        free(*ptr);
    free(original_exec_value_parts);
    free(original_exec_value);

    return original_exec_value_args;
}

TEST_F(DesktopIntegrationTests, modify_desktop_file) {
    // Test SetUp
    fillMinimalAppDir();

    // Test body
    char* desktop_file_path = find_desktop_file(appdir_path.c_str());
    ASSERT_TRUE(desktop_file_path);
    GKeyFile* original_desktop_file = load_desktop_file(desktop_file_path);

    char* original_desktop_file_args = extract_exec_args_from_desktop(original_desktop_file);

    bool res = desktop_integration_modify_desktop_file(appimage_path, appdir_path.c_str());
    ASSERT_TRUE(res);

    GKeyFile* desktop_file = load_desktop_file(desktop_file_path);

    char* tryExecValue = g_key_file_get_string(desktop_file,
                                               G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TRY_EXEC, NULL);
    ASSERT_STREQ(tryExecValue, appimage_path);
    g_free(tryExecValue);

    char* execValue = g_key_file_get_string(desktop_file,
                                            G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);

    ASSERT_TRUE(g_str_has_prefix(execValue, appimage_path));
    ASSERT_TRUE(g_str_has_suffix(execValue, original_desktop_file_args));
    g_free(original_desktop_file_args);
    g_free(execValue);

    char* iconValue = g_key_file_get_string(desktop_file,
                                            G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL);
    char* appimage_path_md5 = appimage_get_md5(appimage_path);
    char* expected_icon_prefix = g_strjoin("", "appimagekit_", appimage_path_md5, "_", NULL);
    ASSERT_TRUE(g_str_has_prefix(iconValue, expected_icon_prefix));
    g_free(expected_icon_prefix);
    g_free(appimage_path_md5);
    g_free(iconValue);

    // Test Clean Up
    g_key_file_free(desktop_file);
    free(desktop_file_path);
}


TEST_F(DesktopIntegrationTests, move_files_to_user_data_dir) {
    // Test SetUp
    fillMinimalAppDir();

    char* md5sum = appimage_get_md5(appimage_path);

    desktop_integration_modify_desktop_file(appimage_path, appdir_path.c_str());
    // Test body
    ASSERT_TRUE(desktop_integration_move_files_to_user_data_dir(appdir_path.c_str(), user_dir_path.c_str(), md5sum));

    /** Validate that the desktop file was copied */
    char* path = g_strjoin("", user_dir_path.c_str(), "/applications/appimagekit_", md5sum, "-echo.desktop",
                           NULL);
    ASSERT_TRUE(g_file_test(path, G_FILE_TEST_EXISTS));
    free(path);

    /** Validate that the icon was copied */
    path = g_strjoin("", user_dir_path.c_str(), "/icons/hicolor/32x32/apps/appimagekit_", md5sum,
                     "_utilities-terminal",
                     NULL);
    ASSERT_TRUE(g_file_test(path, G_FILE_TEST_EXISTS));
    free(path);

    // Test Clean Up
    free(md5sum);
}
