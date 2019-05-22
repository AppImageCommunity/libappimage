#include <glib.h>
#include <glib/gstdio.h>

#include "desktop_integration.h"
#include "appimage_handler.h"
#include "appimage/appimage.h"
#include "libappimage_private.h"

extern const char* vendorprefix;

char* desktop_integration_create_tempdir() {
    GError* error = NULL;
    char* path = g_dir_make_tmp("libappimage-XXXXXX", &error);
    if (error) {
        g_warning("%s", error->message);
        g_error_free(error);
    }

    return path;
}

struct {
    const char* tempdir;
} typedef traverse_handler_extract_relevant_desktop_integration_files_data;

void create_parent_dir(const char* file_name) {
    char* parent_dir = g_path_get_dirname(file_name);
    if (g_mkdir_with_parents(parent_dir, S_IRWXU) == -1)
        g_warning("Unable to create temporary parent dir: %s", parent_dir);

    g_free(parent_dir);
}

void traverse_handler_extract_relevant_desktop_integration_files(void* raw_handler, void* entry_data,
                                                                 void* raw_user_data) {
    appimage_handler* handler = raw_handler;
    traverse_handler_extract_relevant_desktop_integration_files_data* data = raw_user_data;

    char* file_name = handler->get_file_name(handler, entry_data);
    if (g_str_has_suffix(file_name, ".Desktop") ||
        g_str_has_suffix(file_name, ".desktop") ||
        g_str_has_prefix(file_name, "usr/share/icons") ||
        g_str_equal(file_name, ".DirIcon")) {

        char* target = g_strjoin("/", data->tempdir, file_name, NULL);

        create_parent_dir(target);

        char* link = handler->get_file_link(handler, entry_data);
        if (link != NULL) {
            appimage_extract_file_following_symlinks(handler->path, file_name, target);
            free(link);
        } else {
            handler->extract_file(handler, entry_data, target);
        }

        g_free(target);
    }

    g_free(file_name);
}

void desktop_integration_extract_relevant_files(const char* appimage_path, const char* tempdir_path) {
    appimage_handler handler = create_appimage_handler(appimage_path);

    traverse_handler_extract_relevant_desktop_integration_files_data data;
    data.tempdir = tempdir_path;
    handler.traverse(&handler, traverse_handler_extract_relevant_desktop_integration_files, &data);
}

char* find_desktop_file(const char* path) {
    GError* error = NULL;
    GDir* temp_dir = g_dir_open(path, 0, &error);
    if (error != NULL) {
        g_warning("%s\n", error->message);
        return NULL;
    }
    const char* entry = NULL;
    while ((entry = g_dir_read_name(temp_dir)) != NULL) {
        if (g_str_has_suffix(entry, ".Desktop") ||
            g_str_has_suffix(entry, ".desktop")) {
            break;
        }
    }

    char* result = NULL;
    if (entry != NULL)
        result = g_strjoin("/", path, entry, NULL);

    g_dir_close(temp_dir);
    return result;
}

GKeyFile* load_desktop_file(const char* file_path) {
    GError* error = NULL;
    GKeyFile* desktop_file = g_key_file_new();
    g_key_file_load_from_file(desktop_file, file_path,
                              G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error);
    if (error) {
        g_warning("%s\n", error->message);
        g_key_file_free(desktop_file);
        desktop_file = NULL;
        g_error_free(error);
    }

    return desktop_file;
}

bool save_desktop_file(GKeyFile* desktop_file, const char* file_path) {
    GError* error = NULL;

    g_key_file_save_to_file(desktop_file, file_path, &error);
    if (error) {
        g_warning("%s\n", error->message);
        g_error_free(error);
        return false;
    }

    return true;
}

bool move_desktop_file(const char* tempdir_path, const char* user_data_dir, const char* md5sum);

/**
 * Get the path section corresponding to: <theme>/<size>/<category>
 *  "/usr/share/icons/hicolor/22x22/apps/appicon.png"
 *                    |_________________|
 *                            this
 * */
char* extract_icon_path_prefix(const char* path) {
    char** path_parts = g_strsplit(path, "usr/share/icons/", 2);
    char** itr = path_parts;
    char* prefix = NULL;
    if (*itr != NULL) {
        g_free(*itr);
        itr++;
    }

    if (*itr != NULL) {
        prefix = g_path_get_dirname(*itr);
        g_free(*itr);
    }

    g_free(path_parts);
    return prefix;
}

bool desktop_integration_modify_desktop_file(const char* appimage_path, const char* tempdir_path, const char* md5) {
    char* desktop_file_path = find_desktop_file(tempdir_path);
    char* desktop_filename = g_path_get_basename(desktop_file_path);

    if (desktop_file_path == NULL) {
        g_critical("Failed to find desktop file path\n");
        return false;
    }

    if (desktop_filename == NULL) {
        g_critical("Failed to query desktop file filename\n");
        return false;
    }

    GKeyFile* key_file_structure = load_desktop_file(desktop_file_path);

    if (!g_key_file_has_key(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL)) {
        g_critical("Desktop file has no Exec key\n");
        return false;
    }

    // parse [Try]Exec= value, replace executable by AppImage path, append parameters
    // TODO: should respect quoted strings within value

    {
        char* field_value = g_key_file_get_value(key_file_structure, G_KEY_FILE_DESKTOP_GROUP,
                                                 G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);

        // saving a copy for later free() call
        char* original_field_value = field_value;

        char* executable = strsep(&field_value, " ");

        // error handling
        if (executable == NULL) {
            g_warning("Invalid value for Exec= entry in Desktop file\n");
            return false;
        }

        unsigned long new_exec_value_size = strlen(appimage_path) + 1;

        if (field_value != NULL)
            new_exec_value_size += strlen(field_value) + 1;

        gchar* new_exec_value = calloc(new_exec_value_size, sizeof(gchar));

        // build new value
        strcpy(new_exec_value, appimage_path);

        if (field_value != NULL && strlen(field_value) > 0) {
            strcat(new_exec_value, " ");
            strcat(new_exec_value, field_value);
        }

        if (original_field_value != NULL)
            free(original_field_value);

        g_key_file_set_value(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, new_exec_value);

        g_free(new_exec_value);
    }

    // force add a TryExec= key
    // of course we need an absolute path for that
    char* absolute_appimage_path;
    if ((absolute_appimage_path = realpath(appimage_path, NULL)) == NULL)
        absolute_appimage_path = strdup(appimage_path);
    g_key_file_set_value(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TRY_EXEC, absolute_appimage_path);

#ifdef APPIMAGED
    /* If firejail is on the $PATH, then use it to run AppImages */
    if(g_find_program_in_path ("firejail")){
        char *firejail_exec;
        firejail_exec = g_strdup_printf("firejail --env=DESKTOPINTEGRATION=appimaged --noprofile --appimage '%s'", appimage_path);
        g_key_file_set_value(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, firejail_exec);

        gchar *firejail_profile_group = "Desktop Action FirejailProfile";
        gchar *firejail_profile_exec = g_strdup_printf("firejail --env=DESKTOPINTEGRATION=appimaged --private --appimage '%s'", appimage_path);
        gchar *firejail_tryexec = "firejail";
        g_key_file_set_value(key_file_structure, firejail_profile_group, G_KEY_FILE_DESKTOP_KEY_NAME, "Run without sandbox profile");
        g_key_file_set_value(key_file_structure, firejail_profile_group, G_KEY_FILE_DESKTOP_KEY_EXEC, firejail_profile_exec);
        g_key_file_set_value(key_file_structure, firejail_profile_group, G_KEY_FILE_DESKTOP_KEY_TRY_EXEC, firejail_tryexec);
        g_key_file_set_value(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, "Actions", "FirejailProfile;");
    }
#endif

#ifdef APPIMAGED
    /* Add AppImageUpdate desktop action
     * https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s10.html
     * This will only work if AppImageUpdate is on the user's $PATH.
     * TODO: we could have it call this appimaged instance instead of AppImageUpdate and let it
     * figure out how to update the AppImage */
    unsigned long upd_offset = 0;
    unsigned long upd_length = 0;
    if (g_find_program_in_path("AppImageUpdate")) {
        if (appimage_type == 2) {
            if (!appimage_get_elf_section_offset_and_length(appimage_path, ".upd_info", &upd_offset, &upd_length) ||
                upd_offset == 0 || upd_length == 0) {
                fprintf(stderr, "Could not read .upd_info section in AppImage's header\n");
            }
            if (upd_length != 1024) {
#ifdef STANDALONE
                fprintf(stderr,
                    "WARNING: .upd_info length is %lu rather than 1024, this might be a bug in the AppImage\n",
                    upd_length);
#endif
            }
        }
        if (appimage_type == 1) {
            /* If we have a type 1 AppImage, then we hardcode the offset and length */
            upd_offset = 33651; // ISO 9660 Volume Descriptor field
            upd_length = 512; // Might be wrong
        }
#ifdef STANDALONE
        fprintf(stderr, ".upd_info offset: %lu\n", upd_offset);
        fprintf(stderr, ".upd_info length: %lu\n", upd_length);
#endif
        char buffer[3];
        FILE* binary = fopen(appimage_path, "rt");
        if (binary != NULL) {
            /* Check whether the first three bytes at the offset are not NULL */
            fseek(binary, upd_offset, SEEK_SET);
            fread(buffer, 1, 3, binary);
            fclose(binary);
            if ((buffer[0] != 0x00) && (buffer[1] != 0x00) && (buffer[2] != 0x00)) {
                gchar* appimageupdate_group = "Desktop Action AppImageUpdate";
                gchar* appimageupdate_exec = g_strdup_printf("%s %s", "AppImageUpdate", appimage_path);
                g_key_file_set_value(key_file_structure, appimageupdate_group, G_KEY_FILE_DESKTOP_KEY_NAME, "Update");
                g_key_file_set_value(key_file_structure, appimageupdate_group, G_KEY_FILE_DESKTOP_KEY_EXEC,
                                     appimageupdate_exec);
                g_key_file_set_value(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, "Actions", "AppImageUpdate;");
            }
        }
    }
#endif

    {
        // parse desktop files and generate a list of locales representing all localized Name/Icon entries
        // NULL refers to the key without the locale tag
        // the locales for both entry types need to be tracked separately due to a limitation in the GLib API, see
        // the comment below for more information
        gchar* nameLocales[256] = {NULL};
        gchar* iconLocales[256] = {NULL};
        gint nameLocalesCount = 1;
        gint iconLocalesCount = 1;

        {
            // create temporary in-memory copy of the keyfile
            gsize bufsize;
            char* orig_buffer = g_key_file_to_data(key_file_structure, &bufsize, NULL);

            if (orig_buffer == NULL) {
                fprintf(stderr, "Failed to create in-memory copy of desktop file\n");
                return false;
            }

            // need to keep original reference to buffer to be able to free() it later
            char* buffer = orig_buffer;

            // parse line by line
            char* line = NULL;
            while ((line = strsep(&buffer, "\n")) != NULL) {
                const bool is_name_entry = strncmp(line, "Name[", 5) == 0;
                const bool is_icon_entry = strncmp(line, "Icon[", 5) == 0;

                // the only keys for which we're interested in localizations is Name and Icon
                if (!(is_name_entry || is_icon_entry))
                    continue;

                // python: s = split(line, "[")[1]
                char* s = strsep(&line, "[");
                s = strsep(&line, "[");

                // python: locale = split(s, "]")[0]
                char* locale = strsep(&s, "]");

                // create references to the right variables
                gchar** locales = NULL;
                gint* localesCount = NULL;

                if (is_name_entry) {
                    locales = nameLocales;
                    localesCount = &nameLocalesCount;
                } else if (is_icon_entry) {
                    locales = iconLocales;
                    localesCount = &iconLocalesCount;
                }

                // avoid duplicates in list of locales
                bool is_duplicate = false;

                // unfortunately, the list of locales is not sorted, therefore a linear search is required
                // however, this won't have a significant impact on the performance
                // start at index 1, first entry is NULL
                for (int i = 1; i < *localesCount; i++) {
                    if (strcmp(locale, locales[i]) == 0) {
                        is_duplicate = true;
                        break;
                    }
                }

                if (is_duplicate)
                    continue;

                locales[(*localesCount)++] = strdup(locale);
            }

            free(orig_buffer);
        }

        // iterate over locales, check whether name or icon entries exist, and edit accordingly
        for (int i = 0; i < iconLocalesCount; i++) {
            const gchar* locale = iconLocales[i];

            // check whether the key is set at all
            gchar* old_contents = NULL;

            // it's a little annoying that the GLib functions don't simply work with NULL as the locale, that'd
            // make the following if-else construct unnecessary
            if (locale == NULL) {
                old_contents = g_key_file_get_string(
                    key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL
                );
            } else {
                // please note that the following call will return a value even if there is no localized version
                // probably to save one call when you're just interested in getting _some_ value while reading
                // a desktop file
                old_contents = g_key_file_get_locale_string(
                    key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, locale, NULL
                );
            }

            // continue to next key if not set
            if (old_contents == NULL) {
                g_free(old_contents);
                continue;
            }

            // copy key's original contents
            static const gchar old_key[] = "X-AppImage-Old-Icon";

            // append AppImage version
            gchar* basename = g_path_get_basename(old_contents);
            gchar* new_contents = g_strdup_printf("%s_%s_%s", vendorprefix, md5, basename);
            g_free(basename);

            // see comment for above if-else construct
            if (locale == NULL) {
                g_key_file_set_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, old_key, old_contents);
                g_key_file_set_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON,
                                      new_contents);
            } else {
                g_key_file_set_locale_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, old_key, locale,
                                             old_contents);
                g_key_file_set_locale_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON,
                                             locale, new_contents);
            }

            // cleanup
            g_free(old_contents);
            g_free(new_contents);
        }

        char* appimage_version = g_key_file_get_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP,
                                                       "X-AppImage-Version", NULL);
        // check for name entries and append version suffix
        if (appimage_version != NULL) {
            for (int i = 0; i < nameLocalesCount; i++) {
                const gchar* locale = nameLocales[i];

                // check whether the key is set at all
                gchar* old_contents;

                // it's a little annoying that the GLib functions don't simply work with NULL as the locale, that'd
                // make the following if-else construct unnecessary
                if (locale == NULL) {
                    old_contents = g_key_file_get_string(
                        key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL
                    );
                } else {
                    // please note that the following call will return a value even if there is no localized version
                    // probably to save one call when you're just interested in getting _some_ value while reading
                    // a desktop file
                    old_contents = g_key_file_get_locale_string(
                        key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, locale, NULL
                    );
                }

                // continue to next key if not set
                if (old_contents == NULL) {
                    g_free(old_contents);
                    continue;
                }

                gchar* version_suffix = g_strdup_printf(" (%s)", appimage_version);

                // check if version suffix has been appended already
                // this makes sure that the version suffix isn't added more than once
                if (strncmp(old_contents + (strlen(old_contents) - strlen(version_suffix)), version_suffix, strlen(version_suffix)) != 0) {
                    // copy key's original contents
                    static const gchar old_key[] = "X-AppImage-Old-Name";

                    // append AppImage version
                    gchar* new_contents = g_strdup_printf("%s%s", old_contents, version_suffix);

                    // see comment for above if-else construct
                    if (locale == NULL) {
                        g_key_file_set_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, old_key, old_contents);
                        g_key_file_set_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP,
                                              G_KEY_FILE_DESKTOP_KEY_NAME,
                                              new_contents);
                    } else {
                        g_key_file_set_locale_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, old_key, locale,
                                                     old_contents);
                        g_key_file_set_locale_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP,
                                                     G_KEY_FILE_DESKTOP_KEY_NAME, locale, new_contents);
                    }

                    // cleanup
                    g_free(new_contents);
                }

                // cleanup
                g_free(old_contents);
                g_free(version_suffix);
            }
        }

        for (int i = 1; i < iconLocalesCount; i++)
            free(iconLocales[i]);

        for (int i = 1; i < nameLocalesCount; i++)
            free(nameLocales[i]);

        // cleanup
        g_free(appimage_version);
    }

#ifdef APPIMAGED
    {
        gchar *generated_by = g_strdup_printf("Generated by appimaged %s", GIT_COMMIT);
        g_key_file_set_value(key_file_structure, "Desktop Entry", "X-AppImage-Comment", generated_by);
        g_free(generated_by);
    }
#endif
    g_key_file_set_value(key_file_structure, "Desktop Entry", "X-AppImage-Identifier", md5);

    bool save_result = save_desktop_file(key_file_structure, desktop_file_path);

    // Clean Up
    g_key_file_free(key_file_structure);
    free(desktop_filename);
    free(desktop_file_path);
    return save_result;
}


char* read_icon_name_from_desktop_file(const char* tempdir_path, const char* appimage_path_md5);

bool move_app_icons(GSList* path, const char* dir, const char* sum);

GSList* find_app_icons(const char* tempdir_path, char* icon_name);

bool move_diricon_as_app_icon(const char* tempdir_path, const char* user_data_dir, const char* appimage_path_md5,
                              const char* icon_name);

bool desktop_integration_move_files_to_user_data_dir(const char* tempdir_path, const char* user_data_dir,
                                                     const char* appimage_path_md5) {
    // Find application icons (in all sizes)
    char* icon_name = read_icon_name_from_desktop_file(tempdir_path, appimage_path_md5);
    GSList* list = find_app_icons(tempdir_path, icon_name);

    bool res = move_desktop_file(tempdir_path, user_data_dir, appimage_path_md5);

    if (res) {
        if (list != NULL)
            move_app_icons(list, user_data_dir, appimage_path_md5);
        else {
            g_warning("No icons found in AppDir/usr/share/icons. Using .DirIcon as fallback");
            move_diricon_as_app_icon(tempdir_path, user_data_dir, appimage_path_md5, icon_name);
        }
    }

    // TODO: Move MIME-TYPES

    free(icon_name);
    return res;
}

/**
 * Move <tempdir_path>/.DirIcon into <user_data_dir>/icons/hicolor/32x32/apps/<vendorprefix>_<appimage_path_md5>_<icon_name>"
 * This function provides a fallback workflow for AppImage that don't properly include their icons.
 *
 * @param tempdir_path
 * @param user_data_dir
 * @param appimage_path_md5
 * @param icon_name
 */
bool move_diricon_as_app_icon(const char* tempdir_path, const char* user_data_dir, const char* appimage_path_md5,
                              const char* icon_name) {
    bool success = false;
    char* target_dir_path = g_build_path("/", user_data_dir, "icons/hicolor/32x32/apps/", NULL);
    g_mkdir_with_parents(target_dir_path, S_IRWXU);

    char* icon_name_with_extension = g_strjoin("", icon_name, ".png", NULL);
    char* new_icon_name = g_strjoin("_", vendorprefix, appimage_path_md5, icon_name_with_extension, NULL);
    char* target_path = g_build_path("/", target_dir_path, new_icon_name, NULL);

    char* source_path = g_build_path("/", tempdir_path, ".DirIcon", NULL);

    success = move_file(source_path, target_path);
    if (!success)
        g_warning("Unable to move icon file from %s to %s", source_path, target_path);

    g_free(source_path);
    g_free(target_path);
    g_free(new_icon_name);
    g_free(icon_name_with_extension);
    g_free(target_dir_path);

    return success;
}

char* read_icon_name_from_desktop_file(const char* tempdir_path, const char* appimage_path_md5) {
    char* icon_name = NULL;
    char* desktop_file_path = find_desktop_file(tempdir_path);
    GKeyFile* desktop_file = load_desktop_file(desktop_file_path);
    if (desktop_file) {
        char* icon_field_value = g_key_file_get_string(desktop_file,
                                                       G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL);

        char* expected_icon_prefix = g_strjoin("_", vendorprefix, appimage_path_md5, "", NULL);
        if (g_str_has_prefix(icon_field_value, expected_icon_prefix))
            icon_name = strdup(icon_field_value + strlen(expected_icon_prefix));
        else
            icon_name = strdup(icon_field_value);

        g_free(expected_icon_prefix);
        g_free(icon_field_value);
    }

    g_key_file_free(desktop_file);
    g_free(desktop_file_path);
    return icon_name;
}

bool move_icon_file(const char* user_data_dir, const char* appimage_path_md5, const char* path) {
    bool succeed = true;
    char* prefix = extract_icon_path_prefix(path);
    char* file_name = g_path_get_basename(path);
    char* new_file_name = g_strjoin("_", vendorprefix, appimage_path_md5, file_name, NULL);

    char* target_dir_path = g_build_path("/", user_data_dir, "icons", prefix, NULL);
    char* target_path = g_build_path("/", target_dir_path, new_file_name, NULL);

    if (g_mkdir_with_parents(target_dir_path, S_IRWXU) == -1) {
        g_warning("Unable to create dir: %s\n", target_dir_path);
        succeed = false;
    }

    if (!move_file(path, target_path)) {
        g_warning("Unable to move icon to: %s\n", target_path);
        succeed = false;
    }

    g_free(target_path);
    g_free(target_dir_path);
    g_free(new_file_name);
    g_free(file_name);
    g_free(prefix);
    return succeed;
}

/**
 * Look for file icons named <icon_name> in the <tempdir_path>/usr/share/icons.
 *
 * @param tempdir_path
 * @param icon_name
 * @return GSList of char*. Use `g_slist_free_full(list, &g_free);` to free it.
 */
GSList* find_app_icons(const char* tempdir_path, char* icon_name) {
    char* icons_dir_path = g_build_path("/", tempdir_path, "usr/share/icons", NULL);

    GSList* list = NULL;
    GQueue* dirs_queue = g_queue_new();
    g_queue_push_head(dirs_queue, icons_dir_path);

    while (!g_queue_is_empty(dirs_queue)) {
        char* current_dir_path = g_queue_pop_head(dirs_queue);
        GDir* current_dir = g_dir_open(current_dir_path, 0, NULL);

        const char* entry = NULL;
        // Iterate directory entries
        while ((entry = g_dir_read_name(current_dir)) != NULL) {
            char* path = g_build_path("/", current_dir_path, entry, NULL);

            if (g_file_test(path, G_FILE_TEST_IS_DIR))
                g_queue_push_head(dirs_queue, path);
            else {
                if (g_str_has_prefix(entry, icon_name))
                    list = g_slist_append(list, path);
            }

        }

        g_dir_close(current_dir);
        g_free(current_dir_path);
    }

    g_queue_free(dirs_queue);
    return list;
}

/**
 * Move icons files listed in the <icon_files> to the <user_data_dir> keeping the <theme>/<size>/<category>
 * file structure and appends the prefix "<vendorprefix>_<appimage_path_md5>_" to the file name.
 *
 * @param icon_files list of icon files to be moved
 * @param user_data_dir directory where the desktop integration files will be deployed usually "$HOME/.local/shared"
 * @param appimage_path_md5
 * @param icon_name application icon name, extracted from the .desktop file
 * @return true on success otherwise false
 * */
bool move_app_icons(GSList* icon_files, const char* user_data_dir, const char* appimage_path_md5) {
    bool succeed = true;
    for (GSList* itr = icon_files; itr != NULL & succeed; itr = itr->next) {
        const char* path = itr->data;
        succeed = move_icon_file(user_data_dir, appimage_path_md5, path);
    }

    return succeed;
}

/**
 * Move a the application .desktop file to <user_data_dir>/applications/<vendorprefix>_<md5sum>_<desktopfilename>
 * @param tempdir_path
 * @param user_data_dir
 * @param md5sum
 */
bool move_desktop_file(const char* tempdir_path, const char* user_data_dir, const char* md5sum) {
    bool succeed = false;
    char* target_dir_path = g_build_path("/", user_data_dir, "applications", NULL);
    succeed = g_mkdir_with_parents(target_dir_path, S_IRWXU) == 0;

    char* desktop_file_path = find_desktop_file(tempdir_path);
    char* desktop_filename = g_path_get_basename(desktop_file_path);

    char* target_desktop_filename = g_strdup_printf("%s_%s-%s", vendorprefix, md5sum, desktop_filename);
    free(desktop_filename);

    char* target_desktop_file_path = g_build_path("/", target_dir_path, target_desktop_filename, NULL);
    free(target_desktop_filename);
    free(target_dir_path);

    succeed = move_file(desktop_file_path, target_desktop_file_path);

    free(desktop_file_path);
    free(target_desktop_file_path);

    return succeed;
}

void desktop_integration_remove_tempdir(const char* dir_path) {
    GDir* tempdir = NULL;
    GError* error;
    tempdir = g_dir_open(dir_path, 0, &error);
    if (!tempdir) {
        g_warning("%s\n", error->message);
        g_error_free(error);
        return;
    }

    const char* entry;
    while ((entry = g_dir_read_name(tempdir)) != NULL) {
        char* full_path = g_strjoin("/", dir_path, entry, NULL);
        if (g_file_test(full_path, G_FILE_TEST_IS_DIR)) {
            desktop_integration_remove_tempdir(full_path);
        } else
            g_remove(full_path);

        free(full_path);
    }

    g_rmdir(dir_path);
    g_dir_close(tempdir);
}
