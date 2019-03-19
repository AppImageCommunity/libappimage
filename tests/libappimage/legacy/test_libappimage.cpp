#include "appimage/appimage.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>

#include <glib.h>
#include <glib/gstdio.h>

#include <gtest/gtest.h>

#include "fixtures.h"

using namespace std;


class LibAppImageTest : public TestBase {
protected:
    void rm_file(const std::string& path) {
        g_remove(path.c_str());
    }

    bool areIntegrationFilesDeployed(const std::string& path) {
        gchar* sum = appimage_get_md5(path.c_str());

        GDir* dir;
        GError* error = NULL;
        const gchar* filename = NULL;

        char* data_home = xdg_data_home();
        char* apps_path = g_strconcat(data_home, "/applications/", NULL);
        free(data_home);

        bool found = false;
        dir = g_dir_open(apps_path, 0, &error);
        if (dir != NULL) {
            while ((filename = g_dir_read_name(dir))) {
                gchar* m = g_strrstr(filename, sum);

                if (m != NULL)
                    found = true;
            }
            g_dir_close(dir);
        }
        g_free(apps_path);
        g_free(sum);
        return found;
    }
};

TEST_F(LibAppImageTest, appimage_get_type_invalid) {
    ASSERT_EQ(appimage_get_type("/tmp", false), -1);
}

TEST_F(LibAppImageTest, appimage_get_type_on_bare_iso_9660_file) {
    ASSERT_EQ(appimage_get_type(iso_9660_file_path.c_str(), false), -1);
}

TEST_F(LibAppImageTest, appimage_get_type_on_bare_elf_file) {
    ASSERT_EQ(appimage_get_type(elf_file_path.c_str(), false), -1);
}

TEST_F(LibAppImageTest, appimage_get_type_1) {
    ASSERT_EQ(appimage_get_type(appImage_type_1_file_path.c_str(), false), 1);
}

TEST_F(LibAppImageTest, appimage_get_type_on_appimage_type_1_withouth_magic_bytes) {
    ASSERT_EQ(appimage_get_type(appImage_type_1_no_magic_file_path.c_str(), false), 1);
}

TEST_F(LibAppImageTest, appimage_get_type_2) {
    ASSERT_EQ(appimage_get_type(appImage_type_2_file_path.c_str(), false), 2);
}

TEST_F(LibAppImageTest, appimage_unregister_in_system) {
    ASSERT_FALSE(areIntegrationFilesDeployed(appImage_type_1_file_path));
    ASSERT_FALSE(areIntegrationFilesDeployed(appImage_type_2_file_path));
}

TEST_F(LibAppImageTest, appimage_get_md5) {
    std::string pathToTestFile = "/some/fixed/path";

    std::string expected = "972f4824b8e6ea26a55e9af60a285af7";

    gchar* sum = appimage_get_md5(pathToTestFile.c_str());
    EXPECT_EQ(sum, expected);
    g_free(sum);

    unlink(pathToTestFile.c_str());
}

TEST_F(LibAppImageTest, get_md5_invalid_file_path) {
    gchar* sum = appimage_get_md5("");

    ASSERT_TRUE(sum == NULL) << "sum is not NULL";
}

TEST_F(LibAppImageTest, appimage_extract_file_following_symlinks) {
    std::string target_path = tempDir + "/test_libappimage_tmp_file";
    appimage_extract_file_following_symlinks(appImage_type_2_file_path.c_str(), "echo.desktop",
                                             target_path.c_str());

    const char expected[] = ("[Desktop Entry]\n"
                             "Version=1.0\n"
                             "Type=Application\n"
                             "Name=Echo\n"
                             "Comment=Just echo.\n"
                             "Exec=echo %F\n"
                             "Icon=utilities-terminal\n");

    ASSERT_TRUE(g_file_test(target_path.c_str(), G_FILE_TEST_EXISTS));

    std::ifstream file(target_path.c_str(), std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(static_cast<unsigned long>(size));
    if (file.read(buffer.data(), size))
        ASSERT_TRUE(strncmp(expected, buffer.data(), strlen(expected)) == 0);
    else
        FAIL();

    // Clean
    remove(target_path.c_str());
}

TEST_F(LibAppImageTest, appimage_extract_file_following_hardlinks_type_1) {
    const char target_file_path[] = "/tmp/appimage_tmp_file";
    appimage_extract_file_following_symlinks(appImage_type_1_file_path.c_str(),
                                             "AppImageExtract.png", target_file_path);


    // using EXPECT makes sure the free call is executed
    EXPECT_TRUE(g_file_test(target_file_path, G_FILE_TEST_EXISTS));
    EXPECT_TRUE(g_file_test(target_file_path, G_FILE_TEST_IS_REGULAR));

    struct stat stats = {};
    lstat(target_file_path, &stats);
    EXPECT_NE(stats.st_size, 0);

    g_remove(target_file_path);
}

TEST_F(LibAppImageTest, appimage_read_file_into_buffer_following_symlinks_type_2) {
    char* buf = NULL;
    unsigned long bufsize = 0;
    bool rv = appimage_read_file_into_buffer_following_symlinks(appImage_type_2_file_path.c_str(), "echo.desktop", &buf,
                                                                &bufsize);

    // using EXPECT makes sure the free call is executed
    EXPECT_TRUE(rv);
    EXPECT_TRUE(buf != NULL);
    EXPECT_TRUE(bufsize != 0);

    static const char expected[] = ("[Desktop Entry]\n"
                                    "Version=1.0\n"
                                    "Type=Application\n"
                                    "Name=Echo\n"
                                    "Comment=Just echo.\n"
                                    "Exec=echo %F\n"
                                    "Icon=utilities-terminal\n");

    EXPECT_EQ(bufsize, strlen(expected));
    EXPECT_TRUE(buf != NULL && strncmp(expected, buf, bufsize) == 0);
    free(buf);
}

TEST_F(LibAppImageTest, appimage_read_file_into_buffer_following_symlinks_type_1) {
    char* buf = NULL;
    unsigned long bufsize = 0;
    bool rv = appimage_read_file_into_buffer_following_symlinks(appImage_type_1_file_path.c_str(),
                                                                "AppImageExtract.desktop", &buf, &bufsize);

    // using EXPECT makes sure the free call is executed
    EXPECT_TRUE(rv);
    EXPECT_TRUE(buf != NULL);
    EXPECT_TRUE(bufsize != 0);

    static const char expected[] = ("[Desktop Entry]\n"
                                    "Name=AppImageExtract\n"
                                    "Exec=appimageextract\n"
                                    "Icon=AppImageExtract\n"
                                    "Terminal=true\n"
                                    "Type=Application\n"
                                    "Categories=Development;\n"
                                    "Comment=Extract AppImage contents, part of AppImageKit\n"
                                    "StartupNotify=true\n");

    EXPECT_EQ(bufsize, strlen(expected));
    EXPECT_TRUE(buf != NULL && strncmp(expected, buf, bufsize) == 0);
    free(buf);
}

TEST_F(LibAppImageTest, appimage_list_files_false_appimage) {

    char** files = appimage_list_files("/bin/ls");

    char* expected[] = {NULL};

    int i = 0;
    for (; files[i] != NULL && expected[i] != NULL; i++)
        EXPECT_STREQ(files[i], expected[i]);

    appimage_string_list_free(files);

    if (i != 0)
        FAIL();
}

TEST_F(LibAppImageTest, appimage_list_files_type_1) {

    char** files = appimage_list_files(appImage_type_1_file_path.c_str());

    static const char* const expected[] = {
        "usr",
        "usr/bin",
        "usr/lib",
        "AppImageExtract.desktop",
        ".DirIcon",
        "AppImageExtract.png",
        "usr/bin/appimageextract",
        "AppRun",
        "usr/bin/xorriso",
        "usr/lib/libburn.so.4",
        "usr/lib/libisoburn.so.1",
        "usr/lib/libisofs.so.6",
        NULL};

    int i = 0;
    for (; files[i] != NULL && expected[i] != NULL; i++)
        EXPECT_STREQ(files[i], expected[i]);

    appimage_string_list_free(files);
    ASSERT_EQ(i, 12);
}

TEST_F(LibAppImageTest, appimage_list_files_type_2) {

    char** files = appimage_list_files(appImage_type_2_file_path.c_str());
    static const char* const expected[] = {
        ".DirIcon",
        "AppRun",
        "echo.desktop",
        "usr",
        "usr/bin",
        "usr/bin/echo",
        "usr/bin",
        "usr/share",
        "usr/share/applications",
        "usr/share/applications/echo.desktop",
        "usr/share/applications",
        "usr/share",
        "usr",
        "utilities-terminal.svg",
        NULL};

    int i = 0;
    for (; files[i] != NULL && expected[i] != NULL; i++)
        EXPECT_STREQ(files[i], expected[i]);

    appimage_string_list_free(files);
    ASSERT_EQ(i, 14);
}

TEST_F(LibAppImageTest, test_appimage_registered_desktop_file_path_not_registered) {
    EXPECT_TRUE(appimage_registered_desktop_file_path(appImage_type_1_file_path.c_str(), NULL, false) == NULL);
    EXPECT_TRUE(appimage_registered_desktop_file_path(appImage_type_2_file_path.c_str(), NULL, false) == NULL);
}


TEST_F(LibAppImageTest, test_appimage_type1_is_terminal_app) {
    // TODO: add type 1 AppImage with Terminal=false
    EXPECT_EQ(appimage_type1_is_terminal_app(appImage_type_1_file_path.c_str()), 1);
    EXPECT_EQ(appimage_type1_is_terminal_app("/invalid/path"), -1);
}

TEST_F(LibAppImageTest, test_appimage_type2_is_terminal_app) {
    EXPECT_EQ(appimage_type2_is_terminal_app(appImage_type_2_terminal_file_path.c_str()), 1);
    EXPECT_EQ(appimage_type2_is_terminal_app(appImage_type_2_file_path.c_str()), 0);
    EXPECT_EQ(appimage_type2_is_terminal_app("/invalid/path"), -1);
}

TEST_F(LibAppImageTest, test_appimage_is_terminal_app) {
    EXPECT_EQ(appimage_is_terminal_app(appImage_type_1_file_path.c_str()), 1);
    EXPECT_EQ(appimage_is_terminal_app(appImage_type_2_file_path.c_str()), 0);
    // TODO: add type 1 AppImage with Terminal=true
    //EXPECT_EQ(appimage_is_terminal_app(appImage_type_1_terminal_file_path.c_str()), 1);
    EXPECT_EQ(appimage_is_terminal_app(appImage_type_2_terminal_file_path.c_str()), 1);
    EXPECT_EQ(appimage_is_terminal_app("/invalid/path"), -1);
}

// compares whether the size first bytes of two given byte buffers are equal
bool test_compare_bytes(const char* buf1, const char* buf2, int size) {
    for (int i = 0; i < size; i++) {
        if (buf1[i] != buf2[i]) {
            return false;
        }
    }

    return true;
}

TEST_F(LibAppImageTest, appimage_type2_digest_md5) {
    char digest[16];
    char expectedDigest[] = {-75, -71, 106, -93, 122, 114, 7, 127, -40, 10, -115, -82, -73, 115, -19, 1};

    EXPECT_TRUE(appimage_type2_digest_md5(appImage_type_2_file_path.c_str(), digest));
    EXPECT_PRED3(test_compare_bytes, digest, expectedDigest, 16);
}

#ifdef LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED

bool test_appimage_is_registered_in_system(const std::string& pathToAppImage, bool integrateAppImage) {
    if (integrateAppImage) {
        EXPECT_EQ(appimage_register_in_system(pathToAppImage.c_str(), false), 0);
    }

    return appimage_is_registered_in_system(pathToAppImage.c_str());
}

TEST_F(LibAppImageTest, appimage_is_registered_in_system) {
    // make sure tested AppImages are not registered
    appimage_unregister_in_system(appImage_type_1_file_path.c_str(), false);
    appimage_unregister_in_system(appImage_type_2_file_path.c_str(), false);

    // if the test order is false -> true, cleanup isn't necessary

    // type 1 tests
    EXPECT_FALSE(test_appimage_is_registered_in_system(appImage_type_1_file_path, false));
    EXPECT_TRUE(test_appimage_is_registered_in_system(appImage_type_1_file_path, true));

    // type 2 tests
    EXPECT_FALSE(test_appimage_is_registered_in_system(appImage_type_2_file_path, false));
    EXPECT_TRUE(test_appimage_is_registered_in_system(appImage_type_2_file_path, true));

    // cleanup
    appimage_unregister_in_system(appImage_type_1_file_path.c_str(), false);
    appimage_unregister_in_system(appImage_type_2_file_path.c_str(), false);
}

TEST_F(LibAppImageTest, test_appimage_registered_desktop_file_path_type1) {
    EXPECT_TRUE(appimage_type1_register_in_system(appImage_type_1_file_path.c_str(), false));

    char* desktop_file_path = appimage_registered_desktop_file_path(appImage_type_1_file_path.c_str(), NULL, false);

    EXPECT_TRUE(desktop_file_path != NULL);

    free(desktop_file_path);
}

TEST_F(LibAppImageTest, test_appimage_registered_desktop_file_path_type2) {
    EXPECT_TRUE(appimage_type2_register_in_system(appImage_type_2_file_path.c_str(), false));

    char* desktop_file_path = appimage_registered_desktop_file_path(appImage_type_2_file_path.c_str(), NULL, false);

    EXPECT_TRUE(desktop_file_path != NULL);

    free(desktop_file_path);
}

TEST_F(LibAppImageTest, test_appimage_registered_desktop_file_path_type1_precalculated_md5) {
    EXPECT_TRUE(appimage_type1_register_in_system(appImage_type_1_file_path.c_str(), false));

    char* md5 = appimage_get_md5(appImage_type_1_file_path.c_str());
    char* desktop_file_path = appimage_registered_desktop_file_path(appImage_type_1_file_path.c_str(), md5, false);
    free(md5);

    EXPECT_TRUE(desktop_file_path != NULL);

    free(desktop_file_path);
}

TEST_F(LibAppImageTest, test_appimage_registered_desktop_file_path_type2_precalculated_md5) {
    EXPECT_TRUE(appimage_type2_register_in_system(appImage_type_2_file_path.c_str(), false));

    char* md5 = appimage_get_md5(appImage_type_2_file_path.c_str());
    char* desktop_file_path = appimage_registered_desktop_file_path(appImage_type_2_file_path.c_str(), md5, false);
    free(md5);

    EXPECT_TRUE(desktop_file_path != NULL);

    free(desktop_file_path);
}

TEST_F(LibAppImageTest, test_appimage_registered_desktop_file_path_type1_wrong_md5) {
    EXPECT_TRUE(appimage_type1_register_in_system(appImage_type_1_file_path.c_str(), false));

    char* md5 = strdup("abcdefg");
    char* desktop_file_path = appimage_registered_desktop_file_path(appImage_type_1_file_path.c_str(), md5, false);
    free(md5);

    EXPECT_TRUE(desktop_file_path == NULL);

    free(desktop_file_path);
}

TEST_F(LibAppImageTest, test_appimage_registered_desktop_file_path_type2_wrong_md5) {
    EXPECT_TRUE(appimage_type2_register_in_system(appImage_type_2_file_path.c_str(), false));

    char* md5 = strdup("abcdefg");
    char* desktop_file_path = appimage_registered_desktop_file_path(appImage_type_2_file_path.c_str(), md5, false);
    free(md5);

    EXPECT_TRUE(desktop_file_path == NULL);

    free(desktop_file_path);
}

TEST_F(LibAppImageTest, test_appimage_type2_appimage_version) {
    EXPECT_TRUE(appimage_type2_register_in_system(appImage_type_2_versioned_path.c_str(), false));

    char* desktopFilePath = appimage_registered_desktop_file_path(appImage_type_2_versioned_path.c_str(), NULL, false);

    GKeyFile* desktopFile = g_key_file_new();

    GError* error = NULL;

    gboolean loaded = g_key_file_load_from_file(desktopFile, desktopFilePath, G_KEY_FILE_KEEP_TRANSLATIONS, &error);

    if (!loaded) {
        g_key_file_free(desktopFile);
        ADD_FAILURE() << "Failed to read desktop file: " << error->message;
        g_error_free(error);
        return;
    }

    const std::string versionKey = "X-AppImage-Version";
    const std::string oldNameKey = "X-AppImage-Old-Name";

    std::string expectedVersion = "test1234";
    gchar* actualVersion = g_key_file_get_string(desktopFile, G_KEY_FILE_DESKTOP_GROUP, versionKey.c_str(), &error);

    if (actualVersion == NULL) {
        g_key_file_free(desktopFile);
        ADD_FAILURE() << "Failed to get " << versionKey << " key: " << error->message;
        g_error_free(error);
        return;
    }

    EXPECT_EQ(expectedVersion, std::string(actualVersion));

    gchar* oldName = g_key_file_get_string(desktopFile, G_KEY_FILE_DESKTOP_GROUP, oldNameKey.c_str(), &error);

    if (oldName == NULL) {
        g_key_file_free(desktopFile);
        ADD_FAILURE() << "Failed to get " << oldNameKey << " key: " << error->message;
        g_error_free(error);
        return;
    }

    gchar* newName = g_key_file_get_string(desktopFile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, &error);

    if (newName == NULL) {
        g_key_file_free(desktopFile);
        ADD_FAILURE() << "Failed to get " << G_KEY_FILE_DESKTOP_KEY_NAME << " key: " << error->message;
        g_error_free(error);
        return;
    }

    std::string expectedName = std::string(oldName) + " (" + expectedVersion + ")";

    EXPECT_EQ(expectedName, std::string(newName));

    // cleanup
    g_key_file_free(desktopFile);
    if (error != NULL)
        g_error_free(error);
}

TEST_F(LibAppImageTest, appimage_register_in_system_with_type1) {
    ASSERT_EQ(appimage_register_in_system(appImage_type_1_file_path.c_str(), false), 0);

    ASSERT_TRUE(areIntegrationFilesDeployed(appImage_type_1_file_path));

    appimage_unregister_in_system(appImage_type_1_file_path.c_str(), false);
}

TEST_F(LibAppImageTest, appimage_register_in_system_with_type2) {
    ASSERT_EQ(appimage_register_in_system(appImage_type_2_file_path.c_str(), false), 0);

    ASSERT_TRUE(areIntegrationFilesDeployed(appImage_type_2_file_path));

    appimage_unregister_in_system(appImage_type_2_file_path.c_str(), false);
}

TEST_F(LibAppImageTest, appimage_type1_register_in_system) {
    ASSERT_TRUE(appimage_type1_register_in_system(appImage_type_1_file_path.c_str(), false));

    ASSERT_TRUE(areIntegrationFilesDeployed(appImage_type_1_file_path));

    appimage_unregister_in_system(appImage_type_1_file_path.c_str(), false);
}

TEST_F(LibAppImageTest, appimage_type2_register_in_system) {
    EXPECT_TRUE(appimage_type2_register_in_system(appImage_type_2_file_path.c_str(), false));

    EXPECT_TRUE(areIntegrationFilesDeployed(appImage_type_2_file_path));
    appimage_unregister_in_system(appImage_type_2_file_path.c_str(), false);
}

TEST_F(LibAppImageTest, test_try_exec_key_exists_type_1) {
    const std::string& pathToAppImage = appImage_type_1_file_path;

    ASSERT_EQ(appimage_register_in_system(pathToAppImage.c_str(), false), 0);

    GKeyFile* kf = g_key_file_new();

    const char* desktopFilePath = appimage_registered_desktop_file_path(pathToAppImage.c_str(), NULL, false);

    ASSERT_TRUE(g_key_file_load_from_file(kf, desktopFilePath, G_KEY_FILE_NONE, NULL));

    const char* expectedTryExecValue = g_key_file_get_string(kf, G_KEY_FILE_DESKTOP_GROUP,
                                                             G_KEY_FILE_DESKTOP_KEY_TRY_EXEC, NULL);

    EXPECT_EQ(expectedTryExecValue, pathToAppImage);
}

TEST_F(LibAppImageTest, test_try_exec_key_exists_type_2) {
    const std::string& pathToAppImage = appImage_type_2_file_path;

    ASSERT_EQ(appimage_register_in_system(pathToAppImage.c_str(), false), 0);

    GKeyFile* kf = g_key_file_new();

    const char* desktopFilePath = appimage_registered_desktop_file_path(pathToAppImage.c_str(), NULL, false);

    ASSERT_TRUE(g_key_file_load_from_file(kf, desktopFilePath, G_KEY_FILE_NONE, NULL));

    const char* expectedTryExecValue = g_key_file_get_string(kf, G_KEY_FILE_DESKTOP_GROUP,
                                                             G_KEY_FILE_DESKTOP_KEY_TRY_EXEC, NULL);

    EXPECT_EQ(expectedTryExecValue, pathToAppImage);
}

TEST_F(LibAppImageTest, test_appimage_type1_shall_not_integrate) {
    // TODO: add type 1 AppImage with X-AppImage-Integrate=false
    //EXPECT_EQ(appimage_is_terminal_app(appImage_type_1_shall_not_integrate_path.c_str()), 1);
    EXPECT_EQ(appimage_type1_shall_not_be_integrated(appImage_type_1_file_path.c_str()), 0);
//    EXPECT_EQ(appimage_type1_shall_not_be_integrated(appImage_type_2_file_path.c_str()), -1);
    EXPECT_EQ(appimage_type1_shall_not_be_integrated("/invalid/path"), -1);
}

TEST_F(LibAppImageTest, test_appimage_type2_shall_not_integrate) {
//    EXPECT_EQ(appimage_type2_shall_not_be_integrated(appImage_type_1_file_path.c_str()), -1);
    EXPECT_EQ(appimage_type2_shall_not_be_integrated(appImage_type_2_shall_not_integrate_path.c_str()), 1);
    EXPECT_EQ(appimage_type2_shall_not_be_integrated(appImage_type_2_file_path.c_str()), 0);
    EXPECT_EQ(appimage_type2_shall_not_be_integrated("/invalid/path"), -1);
}

TEST_F(LibAppImageTest, test_appimage_shall_not_integrate) {
    EXPECT_EQ(appimage_shall_not_be_integrated(appImage_type_1_file_path.c_str()), 0);
    EXPECT_EQ(appimage_shall_not_be_integrated(appImage_type_2_file_path.c_str()), 0);
    // TODO: add type 1 AppImage with X-AppImage-Integrate=false
    //EXPECT_EQ(appimage_shall_not_be_integrated(appImage_type_1_shall_not_integrate_path.c_str()), 1);
    EXPECT_EQ(appimage_shall_not_be_integrated(appImage_type_2_shall_not_integrate_path.c_str()), 1);
    EXPECT_EQ(appimage_is_terminal_app("/invalid/path"), -1);
}

#ifdef LIBAPPIMAGE_THUMBNAILER_ENABLED

TEST_F(LibAppImageTest, create_thumbnail_appimage_type_1) {
    EXPECT_TRUE(appimage_create_thumbnail(appImage_type_1_file_path.c_str(), false));

    gchar* sum = appimage_get_md5(appImage_type_1_file_path.c_str());

    char* cache_home = xdg_cache_home();
    std::string path = std::string(cache_home)
                       + "/thumbnails/normal/"
                       + std::string(sum) + ".png";

    g_free(cache_home);
    g_free(sum);

    ASSERT_TRUE(g_file_test(path.c_str(), G_FILE_TEST_EXISTS));

    // Clean
    rm_file(path);
}

TEST_F(LibAppImageTest, create_thumbnail_appimage_type_2) {
    EXPECT_TRUE(appimage_create_thumbnail(appImage_type_2_file_path.c_str(), false));

    gchar* sum = appimage_get_md5(appImage_type_2_file_path.c_str());

    char* cache_home = xdg_cache_home();
    std::string path = std::string(cache_home)
                       + "/thumbnails/normal/"
                       + std::string(sum) + ".png";

    g_free(cache_home);
    g_free(sum);

    ASSERT_TRUE(g_file_test(path.c_str(), G_FILE_TEST_EXISTS));

    // Clean
    rm_file(path);
}

#endif // LIBAPPIMAGE_THUMBNAILER_ENABLED

#endif // LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED
