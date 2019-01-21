/**************************************************************************
 *
 * Copyright (c) 2004-18 Simon Peter
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/

#ident "AppImage by Simon Peter, http://appimage.org/"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <gio/gio.h>

#include "squashfuse.h"
#include <squashfs_fs.h>
#include "elf.h"

#include "xdg-basedir.h"
#include "appimage_handler.h"

// own header
#include "appimage/appimage.h"
#include "desktop_integration.h"
#include "type1.h"

#if HAVE_LIBARCHIVE3 == 1 // CentOS
# include <archive3.h>
# include <archive_entry3.h>
#else // other systems
# include <archive.h>
# include <archive_entry.h>
#endif

#include <regex.h>
#include <glob.h>

#include <cairo.h> // To get the size of icons, move_icon_to_destination()

#define FNM_FILE_NAME 2

#define URI_MAX (FILE_MAX * 3 + 8)

char *vendorprefix = "appimagekit";

/* Search and replace on a string, this really should be in Glib */
gchar* replace_str(const gchar const *src, const gchar const *find, const gchar const *replace){
    gchar **split = g_strsplit(src, find, -1);
    gchar *retval = g_strjoinv(replace, split);

    g_strfreev(split);
    return retval;
}

/* Return the md5 hash constructed according to
 * https://specifications.freedesktop.org/thumbnail-spec/thumbnail-spec-latest.html#THUMBSAVE
 * This can be used to identify files that are related to a given AppImage at a given location */
char *appimage_get_md5(const char* path)
{
    char* absolute_path;

    if ((absolute_path = realpath(path, NULL)) == NULL)
        absolute_path = strdup(path);

    gchar *uri = g_filename_to_uri(absolute_path, NULL, NULL);

    free(absolute_path);

    if (uri != NULL)
    {
        GChecksum *checksum;
        checksum = g_checksum_new(G_CHECKSUM_MD5);
        guint8 digest[16];
        gsize digest_len = sizeof (digest);
        g_checksum_update(checksum, (const guchar *) uri, strlen (uri));
        g_checksum_get_digest(checksum, digest, &digest_len);
        g_assert(digest_len == 16);
        gchar *result = g_strdup(g_checksum_get_string(checksum));
        g_checksum_free(checksum);
        g_free(uri);
        return result;
    } else {
        return NULL;
    }
}

/* Return the path of the thumbnail regardless whether it already exists; may be useful because
 * G*_FILE_ATTRIBUTE_THUMBNAIL_PATH only exists if the thumbnail is already there.
 * Check libgnomeui/gnome-thumbnail.h for actually generating thumbnails in the correct
 * sizes at the correct locations automatically; which would draw in a dependency on gdk-pixbuf.
 */
char *get_thumbnail_path(const char *path, char *thumbnail_size, gboolean verbose)
{
    char *md5 = appimage_get_md5(path);
    char *file  = g_strconcat(md5, ".png", NULL);
    char* cache_home = xdg_cache_home();
    gchar *thumbnail_path = g_build_filename(cache_home, "thumbnails", thumbnail_size, file, NULL);
    g_free(cache_home);
    g_free(file);
    g_free(md5);
    return thumbnail_path;
}

/* Move an icon file to the path where a given icon can be installed in $HOME.
 * This is needed because png and xpm icons cannot be installed in a generic
 * location but are only picked up in directories that have the size of
 * the icon as part of their directory name, as specified in the theme.index
 * See https://github.com/AppImage/AppImageKit/issues/258
 */

void move_icon_to_destination(gchar *icon_path, gboolean verbose)
{
    // FIXME: This default location is most likely wrong, but at least the icons with unknown size can go somewhere
    char* data_home = xdg_data_home();
    gchar *dest_dir = g_build_path("/", data_home, "/icons/hicolor/48x48/apps/", NULL);

    if((g_str_has_suffix (icon_path, ".svg")) || (g_str_has_suffix (icon_path, ".svgz"))) {
        g_free(dest_dir);
        dest_dir = g_build_path("/", data_home, "/icons/hicolor/scalable/apps/", NULL);
    }
    g_free(data_home);

    if((g_str_has_suffix (icon_path, ".png")) || (g_str_has_suffix (icon_path, ".xpm"))) {

        cairo_surface_t *image;

        int w = 0;
        int h = 0;

        if(g_str_has_suffix (icon_path, ".xpm")) {
            // TODO: GdkPixbuf has a convenient way to load XPM data. Then you can call
            // gdk_cairo_set_source_pixbuf() to transfer the data to a Cairo surface.
#ifdef STANDALONE
            fprintf(stderr, "XPM size parsing not yet implemented\n");
#endif
        }

        if(g_str_has_suffix (icon_path, ".png")) {
            image = cairo_image_surface_create_from_png(icon_path);
            w = cairo_image_surface_get_width (image);
            h = cairo_image_surface_get_height (image);
            cairo_surface_destroy (image);
        }

        // FIXME: The following sizes are taken from the hicolor icon theme.
        // Probably the right thing to do would be to figure out at runtime which icon sizes are allowable.
        // Or could we put our own index.theme into .local/share/icons/ and have it observed?
        if((w != h) || ((w != 16) && (w != 24) && (w != 32) && (w != 36) && (w != 48) && (w != 64) && (w != 72) && (w != 96) && (w != 128) && (w != 192) && (w != 256) && (w != 512))){
#ifdef STANDALONE
            fprintf(stderr, "%s has nonstandard size w = %i, h = %i; please fix it\n", icon_path, w, h);
#endif
        } else {
            g_free(dest_dir);
            char* data_home = xdg_data_home();
            dest_dir = g_build_path("/", data_home, "/icons/hicolor/", g_strdup_printf("%ix%i", w, h), "/apps/", NULL);
            free(data_home);
        }
    }
    if(verbose)
        fprintf(stderr, "dest_dir %s\n", dest_dir);

    gchar *basename = g_path_get_basename(icon_path);

    gchar* icon_dest_path = g_build_path("/", dest_dir, basename, NULL);

    g_free(basename);
    if(verbose)
        fprintf(stderr, "Move from %s to %s\n", icon_path, icon_dest_path);
    gchar *dirname = g_path_get_dirname(dest_dir);
    if(g_mkdir_with_parents(dirname, 0755)) {
#ifdef STANDALONE
        fprintf(stderr, "Could not create directory: %s\n", dest_dir);
#endif
    }

    g_free(dirname);
    g_free(dest_dir);

    // This is required only for old versions of glib2 and is harmless for newer
    g_type_init();

    GError *error = NULL;
    GFile *icon_file = g_file_new_for_path(icon_path);
    GFile *target_file = g_file_new_for_path(icon_dest_path);
    if(!g_file_move(icon_file, target_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error)){
#ifdef STANDALONE
        fprintf(stderr, "Error moving file: %s\n", error->message);
#endif
        g_error_free(error);
    }
    g_object_unref(icon_file);
    g_object_unref(target_file);
    g_free(icon_dest_path);

}

/* Get filename extension */
static gchar *get_file_extension(const gchar *filename)
{
    gchar **tokens;
    gchar *extension;
    tokens = g_strsplit(filename, ".", 2);
    if (tokens[0] == NULL)
        extension = NULL;
    else
        extension = g_strdup(tokens[1]);
    g_strfreev(tokens);
    return extension;
}

// Read the file in chunks
void squash_extract_inode_to_file(sqfs *fs, sqfs_inode *inode, const gchar *dest) {
    off_t bytes_already_read = 0;
    sqfs_off_t bytes_at_a_time = 64*1024;
    FILE * f;
    f = fopen (dest, "w+");
    if (f == NULL){
#ifdef STANDALONE
        fprintf(stderr, "fopen error\n");
#endif
        return;
    }
    while (bytes_already_read < (*inode).xtra.reg.file_size)
    {
        char buf[bytes_at_a_time];
        if (sqfs_read_range(fs, inode, (sqfs_off_t) bytes_already_read, &bytes_at_a_time, buf) != SQFS_OK) {
#ifdef STANDALONE
            fprintf(stderr, "sqfs_read_range error\n");
#endif
        }
        fwrite(buf, 1, (size_t) bytes_at_a_time, f);
        bytes_already_read = bytes_already_read + bytes_at_a_time;
    }
    fclose(f);
}

/* Find files in the squashfs matching to the regex pattern.
 * Returns a newly-allocated NULL-terminated array of strings.
 * Use g_strfreev() to free it.
 *
 * The following is done within the sqfs_traverse run for performance reaons:
 * 1.) For found files that are in usr/share/icons, install those icons into the system
 * with a custom name that involves the md5 identifier to tie them to a particular
 * AppImage.
 * 2.) For found files that are in usr/share/mime/packages, install those icons into the system
 * with a custom name that involves the md5 identifier to tie them to a particular
 * AppImage.
 */
gchar **squash_get_matching_files_install_icons_and_mime_data(sqfs* fs, char* pattern,
                                                              gchar* desktop_icon_value_original, char* md5,
                                                              gboolean verbose) {
    GPtrArray *array = g_ptr_array_new();
    sqfs_traverse trv;
    sqfs_err err = sqfs_traverse_open(&trv, fs, sqfs_inode_root(fs));
    if (err!= SQFS_OK) {
#ifdef STANDALONE
        fprintf(stderr, "sqfs_traverse_open error\n");
#endif
    }
    while (sqfs_traverse_next(&trv, &err)) {
        if (!trv.dir_end) {
            int r;
            regex_t regex;
            regmatch_t match[2];
            regcomp(&regex, pattern, REG_ICASE | REG_EXTENDED);
            r = regexec(&regex, trv.path, 2, match, 0);
            regfree(&regex);
            sqfs_inode inode;
            if(sqfs_inode_get(fs, &inode, trv.entry.inode)) {
#ifdef STANDALONE
                fprintf(stderr, "sqfs_inode_get error\n");
#endif
            }
            if(r == 0){
                // We have a match
                if(verbose)
                    fprintf(stderr, "squash_get_matching_files found: %s\n", trv.path);
                g_ptr_array_add(array, g_strdup(trv.path));
                gchar *dest = NULL;
                if(inode.base.inode_type == SQUASHFS_REG_TYPE) {
                    if(g_str_has_prefix(trv.path, "usr/share/icons/") || g_str_has_prefix(trv.path, "usr/share/pixmaps/") || (g_str_has_prefix(trv.path, "usr/share/mime/") && g_str_has_suffix(trv.path, ".xml"))){
                        char* data_home = xdg_data_home();
                        gchar *path = replace_str(trv.path, "usr/share", data_home);
                        free(data_home);
                        char *dest_dirname = g_path_get_dirname(path);
                        g_free(path);
                        gchar *base_name = g_path_get_basename(trv.path);
                        gchar *dest_basename = g_strdup_printf("%s_%s_%s", vendorprefix, md5, base_name);

                        dest = g_build_path("/", dest_dirname, dest_basename, NULL);

                        g_free(base_name);
                        g_free(dest_basename);
                    }
                    /* According to https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html#install_icons
                     * share/pixmaps is ONLY searched in /usr but not in $XDG_DATA_DIRS and hence $HOME and this seems to be true at least in XFCE */
                    if(g_str_has_prefix (trv.path, "usr/share/pixmaps/")){
                        gchar *dest_basename = g_strdup_printf("%s_%s_%s", vendorprefix, md5, g_path_get_basename(trv.path));

                        dest = g_build_path("/", "/tmp", dest_basename, NULL);

                        g_free(dest_basename);
                    }
                    /* Some AppImages only have the icon in their root directory, so we have to get it from there */
                    if((g_str_has_prefix(trv.path, desktop_icon_value_original)) && (! strstr(trv.path, "/")) && ( (g_str_has_suffix(trv.path, ".png")) || (g_str_has_suffix(trv.path, ".xpm")) || (g_str_has_suffix(trv.path, ".svg")) || (g_str_has_suffix(trv.path, ".svgz")))){
                        gchar *ext = get_file_extension(trv.path);
                        gchar *dest_basename = g_strdup_printf("%s_%s_%s.%s", vendorprefix, md5, desktop_icon_value_original, ext);

                        dest = g_build_path("/", "/tmp", dest_basename, NULL);

                        g_free(dest_basename);
                        g_free(ext);
                    }

                    if(dest){
                        if(verbose)
                            fprintf(stderr, "install: %s\n", dest);

                        gchar *dirname = g_path_get_dirname(dest);
                        if(g_mkdir_with_parents(dirname, 0755)) {
#ifdef STANDALONE
                            fprintf(stderr, "Could not create directory: %s\n", dirname);
#endif
                        }

                        g_free(dirname);

                        squash_extract_inode_to_file(fs, &inode, dest);

                        chmod (dest, 0644);

                        if(verbose)
                            fprintf(stderr, "Installed: %s\n", dest);

                        // If we were unsure about the size of an icon, we temporarily installed
                        // it to /tmp and now move it into the proper place
                        if(g_str_has_prefix (dest, "/tmp/")) {
                            move_icon_to_destination(dest, verbose);
                        }

                        g_free(dest);
                    }
                }
            }
        }
    }
    g_ptr_array_add(array, NULL);
    if (err) {
#ifdef STANDALONE
        fprintf(stderr, "sqfs_traverse_next error\n");
#endif
    }
    sqfs_traverse_close(&trv);
    return (gchar **) g_ptr_array_free(array, FALSE);
}



/**
 * Lookup a given <path> in <fs>. If the path points to a symlink it is followed until a regular file is found.
 * This method is aware of symlink loops and will fail properly in such case.
 * @param fs
 * @param path
 * @param inode [RETURN PARAMETER] will be filled with a regular file inode. It cannot be NULL
 * @return succeed true if the file is found, otherwise false
 */
bool sqfs_lookup_path_resolving_symlinks(sqfs* fs, char* path, sqfs_inode* inode) {
    g_assert(fs != NULL);
    g_assert(inode != NULL);

    bool found = false;
    sqfs_inode root_inode;
    sqfs_err err = sqfs_inode_get(fs, &root_inode, fs->sb.root_inode);
    if (err != SQFS_OK) {
#ifdef STANDALONE
        g_warning("sqfs_inode_get root inode error\n");
#endif
        return false;
    }

    *inode = root_inode;
    err = sqfs_lookup_path(fs, inode, path, &found);

    if (!found) {
#ifdef STANDALONE
        g_warning("sqfs_lookup_path path not found: %s\n", path);
#endif
        return false;
    }

    if (err != SQFS_OK) {
#ifdef STANDALONE
        g_warning("sqfs_lookup_path error: %s\n", path);
#endif

        return false;
    }

    // Save visited inode numbers to prevent loops
    GSList* inodes_visited = g_slist_append(NULL, (gpointer) inode->base.inode_number);

    while (inode->base.inode_type == SQUASHFS_SYMLINK_TYPE || inode->base.inode_type == SQUASHFS_LSYMLINK_TYPE) {
        // Read symlink
        size_t size;
        // read twice, once to find out right amount of memory to allocate
        err = sqfs_readlink(fs, inode, NULL, &size);
        if (err != SQFS_OK) {
#ifdef STANDALONE
            fprintf(stderr, "sqfs_readlink error: %s\n", path);
#endif
            g_slist_free(inodes_visited);
            return false;
        }

        char symlink_target_path[size];
        // then to populate the buffer
        err = sqfs_readlink(fs, inode, symlink_target_path, &size);
        if (err != SQFS_OK) {
#ifdef STANDALONE
            g_warning("sqfs_readlink error: %s\n", path);
#endif
            g_slist_free(inodes_visited);
            return false;
        }

        // lookup symlink target path
        *inode = root_inode;
        err = sqfs_lookup_path(fs, inode, symlink_target_path, &found);

        if (!found) {
#ifdef STANDALONE
            g_warning("sqfs_lookup_path path not found: %s\n", symlink_target_path);
#endif
            g_slist_free(inodes_visited);
            return false;
        }

        if (err != SQFS_OK) {
#ifdef  STANDALONE
            g_warning("sqfs_lookup_path error: %s\n", symlink_target_path);
#endif
            g_slist_free(inodes_visited);
            return false;
        }

        // check if we felt into a loop
        if (g_slist_find(inodes_visited, (gconstpointer) inode->base.inode_number)) {
#ifdef STANDALONE
            g_warning("Symlinks loop found while trying to resolve: %s", path);
#endif
            g_slist_free(inodes_visited);
            return false;
        } else
            inodes_visited = g_slist_append(inodes_visited, (gpointer) inode->base.inode_number);
    }

    g_slist_free(inodes_visited);
    return true;
}

/**
 * Read a regular <inode> from <fs> in chunks of <buf_size> and merge them into one.
 *
 * @param fs
 * @param inode
 * @param buffer [RETURN PARAMETER]
 * @param buffer_size [RETURN PARAMETER]
 * @return succeed true, buffer pointing to the memory, buffer_size holding the actual size in memory
 *          if all was ok. Otherwise succeed false, buffer pointing NULL and buffer_size = 0.
 *          The buffer MUST BE FREED using g_free().
 */
bool sqfs_read_regular_inode(sqfs* fs, sqfs_inode *inode, char **buffer, off_t *buffer_size) {
    GSList *blocks = NULL;

    off_t bytes_already_read = 0;
    unsigned long read_buf_size = 256*1024;

    // This has a double role in sqfs_read_range it's used to set the max_bytes_to_be_read and
    // after complete it's set to the number ob bytes actually read.
    sqfs_off_t size = 0;
    sqfs_err err;

    // Read chunks until the end of the file.
    do {
        size = read_buf_size;
        char* buf_read = malloc(sizeof(char) * size);
        if (buf_read != NULL) {
            err = sqfs_read_range(fs, inode, (sqfs_off_t) bytes_already_read, &size, buf_read);
            if (err != SQFS_OK) {
#ifdef STANDALONE
                g_warning("sqfs_read_range failed\n");
#endif
            }
            else
                blocks = g_slist_append(blocks, buf_read);
            bytes_already_read += size;
        } else { // handle not enough memory properly
#ifdef STANDALONE
            g_warning("sqfs_read_regular_inode: Unable to allocate enough memory.\n");
#endif
            err = SQFS_ERR;
        }
    } while ( (err == SQFS_OK) && (size == read_buf_size) );


    bool succeed = false;
    *buffer_size = 0;
    *buffer = NULL;

    if (err == SQFS_OK) {
        // Put all the memory blocks together
        guint length = g_slist_length(blocks);

        *buffer = malloc(sizeof(char) * bytes_already_read);
        if (*buffer != NULL) { // Prevent crash if the
            GSList *ptr = blocks;
            for (int i = 0; i < (length-1); i ++) {
                memcpy(*buffer + (i*read_buf_size), ptr->data, read_buf_size);
                ptr = ptr->next;
            }

            memcpy(*buffer + ((length-1)*read_buf_size), ptr->data, (size_t) size);

            succeed = true;
            *buffer_size = bytes_already_read;
        } else { // handle not enough memory properly
#ifdef STANDALONE
            g_warning("sqfs_read_regular_inode: Unable to allocate enough memory.\n");
#endif
            succeed = false;
        }
    }

    g_slist_free_full(blocks, &g_free);
    return  succeed;
}

/**
 * Loads a desktop file from <fs> into an empty GKeyFile structure. In case of symlinks they are followed.
 * This function is capable of detecting loops and will return false in such cases.
 *
 * @param fs
 * @param path
 * @param key_file_structure [OUTPUT PARAMETER]
 * @param verbose
 * @return true if all when ok, otherwise false.
 */
gboolean g_key_file_load_from_squash(sqfs* fs, char* path, GKeyFile* key_file_structure, gboolean verbose) {
    sqfs_inode inode;
    if (!sqfs_lookup_path_resolving_symlinks(fs, path, &inode))
        return false;

    gboolean success = false;
    if (inode.base.inode_type == SQUASHFS_REG_TYPE || inode.base.inode_type == SQUASHFS_LREG_TYPE ) {
        char* buf = NULL;
        off_t buf_size;
        sqfs_read_regular_inode(fs, &inode, &buf, &buf_size);
        if (buf != NULL) {
            success = g_key_file_load_from_data(key_file_structure, buf, (gsize) buf_size,
                                                G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL);

            g_free(buf);
        } else
            success = false;
    }

    return success;
}

gchar *build_installed_desktop_file_path(gchar* md5, gchar* desktop_filename) {
    gchar *partial_path;
    partial_path = g_strdup_printf("applications/appimagekit_%s-%s", md5, desktop_filename);

    char *data_home = xdg_data_home();
    gchar *destination = g_build_filename(data_home, partial_path, NULL);
    g_free(data_home);

    g_free(partial_path);

    return destination;
}

/* Write a modified desktop file to disk that points to the AppImage */
bool write_edited_desktop_file(GKeyFile *key_file_structure, const char* appimage_path, gchar* desktop_filename, int appimage_type, char *md5, gboolean verbose) {
    if(!g_key_file_has_key(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL)){
#ifdef STANDALONE
        fprintf(stderr, "Desktop file has no Exec key\n");
#endif
        return false;
    }

    // parse [Try]Exec= value, replace executable by AppImage path, append parameters
    // TODO: should respect quoted strings within value

    {
        char* field_value = g_key_file_get_value(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);

        // saving a copy for later free() call
        char* original_field_value = field_value;

        char* executable = strsep(&field_value, " ");

        // error handling
        if (executable == NULL) {
#ifdef STANDALONE
            fprintf(stderr, "Invalid value for Exec= entry in Desktop file\n");
#endif
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
    g_key_file_set_value(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TRY_EXEC, appimage_path);

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
    if(g_find_program_in_path ("AppImageUpdate")){
        if(appimage_type == 2){
            if (!appimage_get_elf_section_offset_and_length(appimage_path, ".upd_info", &upd_offset, &upd_length) || upd_offset == 0 || upd_length == 0) {
                fprintf(stderr, "Could not read .upd_info section in AppImage's header\n");
            }
            if(upd_length != 1024) {
#ifdef STANDALONE
                fprintf(stderr,
                    "WARNING: .upd_info length is %lu rather than 1024, this might be a bug in the AppImage\n",
                    upd_length);
#endif
            }
        }
        if(appimage_type == 1){
            /* If we have a type 1 AppImage, then we hardcode the offset and length */
            upd_offset = 33651; // ISO 9660 Volume Descriptor field
            upd_length = 512; // Might be wrong
        }
#ifdef STANDALONE
        fprintf(stderr, ".upd_info offset: %lu\n", upd_offset);
        fprintf(stderr, ".upd_info length: %lu\n", upd_length);
#endif
        char buffer[3];
        FILE *binary = fopen(appimage_path, "rt");
        if (binary != NULL)
        {
            /* Check whether the first three bytes at the offset are not NULL */
            fseek(binary, upd_offset, SEEK_SET);
            fread(buffer, 1, 3, binary);
            fclose(binary);
            if((buffer[0] != 0x00) && (buffer[1] != 0x00) && (buffer[2] != 0x00)){
                gchar *appimageupdate_group = "Desktop Action AppImageUpdate";
                gchar *appimageupdate_exec = g_strdup_printf("%s %s", "AppImageUpdate", appimage_path);
                g_key_file_set_value(key_file_structure, appimageupdate_group, G_KEY_FILE_DESKTOP_KEY_NAME, "Update");
                g_key_file_set_value(key_file_structure, appimageupdate_group, G_KEY_FILE_DESKTOP_KEY_EXEC, appimageupdate_exec);
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
                g_key_file_set_locale_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, old_key, locale, old_contents);
                g_key_file_set_locale_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, locale, new_contents);
            }

            // cleanup
            g_free(old_contents);
            g_free(new_contents);
        }

        char* appimage_version = g_key_file_get_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, "X-AppImage-Version", NULL);
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

                gchar* version_suffix = g_strdup_printf("(%s)", appimage_version);

                // check if version suffix has been appended already
                // this makes sure that the version suffix isn't added more than once
                if (strlen(version_suffix) > strlen(old_contents) && strcmp(old_contents + (strlen(old_contents) - strlen(version_suffix)), version_suffix) != 0) {
                    // copy key's original contents
                    static const gchar old_key[] = "X-AppImage-Old-Name";

                    // append AppImage version
                    gchar* new_contents = g_strdup_printf("%s %s", old_contents, version_suffix);

                    // see comment for above if-else construct
                    if (locale == NULL) {
                        g_key_file_set_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, old_key, old_contents);
                        g_key_file_set_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, new_contents);
                    } else {
                        g_key_file_set_locale_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, old_key, locale, old_contents);
                        g_key_file_set_locale_string(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, locale, new_contents);
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
#ifdef STANDALONE
    fprintf(stderr, "Installing desktop file\n");
#endif
    if(verbose) {
        gchar *buf  = g_key_file_to_data(key_file_structure, NULL, NULL);
        fprintf(stderr, "%s", buf);
        g_free(buf);
    }

    /* https://specifications.freedesktop.org/menu-spec/menu-spec-latest.html#paths says:
     *
     * $XDG_DATA_DIRS/applications/
     * When two desktop entries have the same name, the one appearing earlier in the path is used.
     *
     * --
     *
     * https://developer.gnome.org/integration-guide/stable/desktop-files.html.en says:
     *
     * Place this file in the /usr/share/applications directory so that it is accessible
     * by everyone, or in ~/.local/share/applications if you only wish to make it accessible
     * to a single user. Which is used should depend on whether your application is
     * installed systemwide or into a user's home directory. GNOME monitors these directories
     * for changes, so simply copying the file to the right location is enough to register it
     * with the desktop.
     *
     * Note that the ~/.local/share/applications location is not monitored by versions of GNOME
     * prior to version 2.10 or on Fedora Core Linux, prior to version 2.8.
     * These versions of GNOME follow the now-deprecated vfolder standard,
     * and so desktop files must be installed to ~/.gnome2/vfolders/applications.
     * This location is not supported by GNOME 2.8 on Fedora Core nor on upstream GNOME 2.10
     * so for maximum compatibility with deployed desktops, put the file in both locations.
     *
     * Note that the KDE Desktop requires one to run kbuildsycoca to force a refresh of the menus.
     *
     * --
     *
     * https://specifications.freedesktop.org/menu-spec/menu-spec-latest.html says:
     *
     * To prevent that a desktop entry from one party inadvertently cancels out
     * the desktop entry from another party because both happen to get the same
     * desktop-file id it is recommended that providers of desktop-files ensure
     * that all desktop-file ids start with a vendor prefix.
     * A vendor prefix consists of [a-zA-Z] and is terminated with a dash ("-").
     * For example, to ensure that GNOME applications start with a vendor prefix of "gnome-",
     * it could either add "gnome-" to all the desktop files it installs
     * in datadir/applications/ or it could install desktop files in a
     * datadir/applications/gnome subdirectory.
     *
     * --
     *
     * https://specifications.freedesktop.org/desktop-entry-spec/latest/ape.html says:
     * The desktop file ID is the identifier of an installed desktop entry file.
     *
     * To determine the ID of a desktop file, make its full path relative
     * to the $XDG_DATA_DIRS component in which the desktop file is installed,
     * remove the "applications/" prefix, and turn '/' into '-'.
     * For example /usr/share/applications/foo/bar.desktop has the desktop file ID
     * foo-bar.desktop.
     * If multiple files have the same desktop file ID, the first one in the
     * $XDG_DATA_DIRS precedence order is used.
     * For example, if $XDG_DATA_DIRS contains the default paths
     * /usr/local/share:/usr/share, then /usr/local/share/applications/org.foo.bar.desktop
     * and /usr/share/applications/org.foo.bar.desktop both have the same desktop file ID
     * org.foo.bar.desktop, but only the first one will be used.
     *
     * --
     *
     * https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s07.html says:
     *
     * The application must name its desktop file in accordance with the naming
     * recommendations in the introduction section (e.g. the filename must be like
     * org.example.FooViewer.desktop). The application must have a D-Bus service
     * activatable at the well-known name that is equal to the desktop file name
     * with the .desktop portion removed (for our example, org.example.FooViewer).
     *
     * --
     *
     * Can it really be that no one thought about having multiple versions of the same
     * application installed? What are we supposed to do if we want
     * a) have desktop files installed by appimaged not interfere with desktop files
     *    provided by the system, i.e., if an application is installed in the system
     *    and the user also installs the AppImage, then both should be available to the user
     * b) both should be D-Bus activatable
     * c) the one installed by appimaged should have an AppImage vendor prefix to make
     *    it easy to distinguish it from system- or upstream-provided ones
     */

    /* FIXME: The following is most likely not correct; see the comments above.
     * Open a GitHub issue or send a pull request if you would like to propose asolution. */
    /* TODO: Check for consistency of the id with the AppStream file, if it exists in the AppImage */
    gchar *destination = build_installed_desktop_file_path(md5, desktop_filename);

    /* When appimaged sees itself, then do nothing here */
    if(strcmp ("appimaged.desktop", desktop_filename) == 0) {
        g_free(destination);
#ifdef STANDALONE
        fprintf(stderr, "appimaged's desktop file found -- not installing desktop file for myself\n");
#endif
        return true;
    }

    if(verbose)
        fprintf(stderr, "install: %s\n", destination);

    gchar *dirname = g_path_get_dirname(destination);
    if(g_mkdir_with_parents(dirname, 0755)) {
#ifdef STANDALONE
        fprintf(stderr, "Could not create directory: %s\n", dirname);
#endif
    }
    g_free(dirname);

    // g_key_file_save_to_file(key_file_structure, destination, NULL);
    // g_key_file_save_to_file is too new, only since 2.40
    /* Write config file on disk */
    gsize length;
    gchar *buf;
    GIOChannel *file;
    buf = g_key_file_to_data(key_file_structure, &length, NULL);
    file = g_io_channel_new_file(destination, "w", NULL);
    g_io_channel_write_chars(file, buf, length, NULL, NULL);
    g_io_channel_shutdown(file, TRUE, NULL);
    g_io_channel_unref(file);

    g_free(buf);

    /* GNOME shows the icon and name on the desktop file only if it is executable */
    chmod(destination, 0755);

    g_free(destination);

    return true;
}

bool appimage_type1_get_desktop_filename_and_key_file(struct archive** a, gchar** desktop_filename, GKeyFile** key_file) {
    // iterate over all files ("entries") in the archive
    // looking for a file with .desktop extension in the root directory

    // must not be freed
    struct archive_entry* entry;

    gchar* filename;

    for (;;) {
        int r = archive_read_next_header(*a, &entry);

        if (r == ARCHIVE_EOF) {
            return false;
        }

        if (r != ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(*a));
            return false;
        }

        /* Skip all but regular files; FIXME: Also handle symlinks correctly */
        if (archive_entry_filetype(entry) != AE_IFREG)
            continue;

        filename = replace_str(archive_entry_pathname(entry), "./", "");

        /* Get desktop file(s) in the root directory of the AppImage and act on it in one go */
        if ((g_str_has_suffix(filename, ".desktop") && (NULL == strstr(filename, "/")))) {
#ifdef STANDALONE
            fprintf(stderr, "Got root desktop: %s\n", filename);
#endif

            const void* buff;

            size_t size = 1024 * 1024;
            int64_t offset = 0;

            r = archive_read_data_block(*a, &buff, &size, &offset);

            if (r == ARCHIVE_EOF) {
                // cleanup
                g_free(filename);

                return true;
            }

            if (r != ARCHIVE_OK) {
                fprintf(stderr, "%s", archive_error_string(*a));
                break;
            }

            *desktop_filename = g_path_get_basename(filename);

            // a structure that will hold the information from the desktop file
            *key_file = g_key_file_new();

            gboolean success = g_key_file_load_from_data(*key_file, buff, size,
                G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL);

            if (!success) {
                // cleanup
                g_free(key_file);
                key_file = NULL;

                break;
            }

            // cleanup
            g_free(filename);

            return true;
        }
    }

    g_free(filename);

    return false;
}

/* Register a type 1 AppImage in the system
 * DEPRECATED, it should be removed ASAP
 * */
bool appimage_type1_register_in_system(const char *path, bool verbose)
{
    return appimage_register_in_system(path, verbose) == 0;
}

bool appimage_type2_get_desktop_filename_and_key_file(sqfs* fs, gchar** desktop_filename, gchar* md5, GKeyFile** key_file, gboolean verbose) {
    /* TOOO: Change so that only one run of squash_get_matching_files is needed in total,
     * this should hopefully improve performance */

    /* Get desktop file(s) in the root directory of the AppImage */
    // Only in root dir
    gchar** str_array = squash_get_matching_files_install_icons_and_mime_data(fs, "(^[^/]*?.desktop$)", "", md5, verbose);

    bool errored = false;

    // gchar **str_array = squash_get_matching_files(&fs, "(^.*?.desktop$)", md5, verbose); // Not only there
    /* Work trough the NULL-terminated array of strings */
    for (int i = 0; str_array[i]; ++i) {
#ifdef STANDALONE
        fprintf(stderr, "Got root desktop: %s\n", str_array[i]);
#endif

        if (!g_key_file_load_from_squash(fs, str_array[i], *key_file, verbose))
            errored = true;
        else
            *desktop_filename = g_path_get_basename(str_array[i]);
    }

    /* Free the NULL-terminated array of strings and its contents */
    g_strfreev(str_array);

    return !errored;
}

/* Register a type 2 AppImage in the system
 * DEPRECATED it should be removed ASAP
 * */
bool appimage_type2_register_in_system(const char *path, bool verbose) {
    return appimage_register_in_system(path, verbose) == 0;
}

int appimage_type1_is_terminal_app(const char* path) {
    // check if file exists
    if (!g_file_test(path, G_FILE_TEST_IS_REGULAR))
        return -1;

    // check if file is of correct type
    if (appimage_get_type(path, false) != 1)
        return -1;

    char* md5 = appimage_get_md5(path);

    if (md5 == NULL)
        return -1;

    // open ISO9660 image using libarchive
    struct archive *a = archive_read_new();
    archive_read_support_format_iso9660(a);

    // libarchive status int -- passed to called functions
    int r;

    if ((r = archive_read_open_filename(a, path, 10240)) != ARCHIVE_OK) {
        // cleanup
        free(md5);
        archive_read_free(a);

        return -1;
    }
    // search image for root desktop file, and read it into key file structure so it can be edited eventually
    gchar *desktop_filename = NULL;
    GKeyFile *key_file = NULL;

    if (!appimage_type1_get_desktop_filename_and_key_file(&a, &desktop_filename, &key_file)) {
        // cleanup
        free(md5);
        archive_read_free(a);
        g_free(desktop_filename);
        g_key_file_free(key_file);

        return -1;
    }

    // validate that both have been set to a non-NULL value
    if (desktop_filename == NULL || key_file == NULL) {
        // cleanup
        free(md5);
        archive_read_free(a);
        g_free(desktop_filename);
        g_key_file_free(key_file);

        return -1;
    }

    GError *error = NULL;
    gboolean rv = g_key_file_get_boolean(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TERMINAL, &error);

    // cleanup
    free(md5);
    archive_read_free(a);
    g_free(desktop_filename);
    g_key_file_free(key_file);

    int result;

    if (!rv) {
        // if the key file hasn't been found and the error is not set to NOT_FOUND, return an error
        if (error != NULL && error->code != G_KEY_FILE_ERROR_KEY_NOT_FOUND)
            result = -1;
        else
            result = 0;
    } else {
        result = 1;
    }

    if (error != NULL)
        g_error_free(error);

    return result;
};

int appimage_type2_is_terminal_app(const char* path) {
    // check if file exists
    if (!g_file_test(path, G_FILE_TEST_IS_REGULAR))
        return -1;

    // check if file is of correct type
    if (appimage_get_type(path, false) != 2)
        return -1;

    char* md5 = appimage_get_md5(path);

    if (md5 == NULL)
        return -1;

    ssize_t fs_offset = appimage_get_elf_size(path);

    // error check
    if (fs_offset < 0)
        return -1;

    sqfs fs;

    sqfs_err err = sqfs_open_image(&fs, path, (size_t) fs_offset);

    if (err != SQFS_OK) {
        free(md5);
        sqfs_destroy(&fs);
        return -1;
    }

    gchar* desktop_filename = NULL;

    // a structure that will hold the information from the desktop file
    GKeyFile* key_file = g_key_file_new();

    if (!appimage_type2_get_desktop_filename_and_key_file(&fs, &desktop_filename, md5, &key_file, false)) {
        // cleanup
        free(md5);
        free(desktop_filename);
        sqfs_destroy(&fs);
        g_key_file_free(key_file);

        return -1;
    }

    // validate that both have been set to a non-NULL value
    if (desktop_filename == NULL || key_file == NULL) {
        // cleanup
        free(md5);
        sqfs_destroy(&fs);
        g_free(desktop_filename);
        g_key_file_free(key_file);

        return -1;
    }

    // no longer used
    free(md5);

    GError *error = NULL;
    gboolean rv = g_key_file_get_boolean(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TERMINAL, &error);

    // cleanup
    free(desktop_filename);
    sqfs_destroy(&fs);
    g_key_file_free(key_file);

    int result;

    if (!rv) {
        // if the key file hasn't been found and the error is not set to NOT_FOUND, return an error
        if (error != NULL && error->code != G_KEY_FILE_ERROR_KEY_NOT_FOUND)
            result = -1;
        else
            result = 0;
    } else {
        result = 1;
    }

    if (error != NULL)
        g_error_free(error);

    return result;
};

/*
 * Checks whether an AppImage's desktop file has set Terminal=true.
 * Useful to check whether the author of an AppImage doesn't want it to be integrated.
 *
 * Returns >0 if set, 0 if not set, <0 on errors.
 */
int appimage_is_terminal_app(const char *path) {
    // check if file exists
    if (!g_file_test(path, G_FILE_TEST_IS_REGULAR))
        return -1;

    int type = appimage_get_type(path, false);

    switch (type) {
        case 1:
            return appimage_type1_is_terminal_app(path);
        case 2:
            return appimage_type2_is_terminal_app(path);
        default:
            return -1;
    }
}

int appimage_type1_shall_not_be_integrated(const char* path) {
    // check if file exists
    if (!g_file_test(path, G_FILE_TEST_IS_REGULAR))
        return -1;

    // check if file is of correct type
    if (appimage_get_type(path, false) != 1)
        return -1;

    char* md5 = appimage_get_md5(path);

    if (md5 == NULL)
        return -1;

    // open ISO9660 image using libarchive
    struct archive *a = archive_read_new();
    archive_read_support_format_iso9660(a);

    // libarchive status int -- passed to called functions
    int r;

    if ((r = archive_read_open_filename(a, path, 10240)) != ARCHIVE_OK) {
        // cleanup
        free(md5);
        archive_read_free(a);

        return -1;
    }
    // search image for root desktop file, and read it into key file structure so it can be edited eventually
    gchar *desktop_filename = NULL;
    GKeyFile *key_file = NULL;

    if (!appimage_type1_get_desktop_filename_and_key_file(&a, &desktop_filename, &key_file)) {
        // cleanup
        free(md5);
        archive_read_free(a);
        g_free(desktop_filename);
        g_key_file_free(key_file);

        return -1;
    }

    // validate that both have been set to a non-NULL value
    if (desktop_filename == NULL || key_file == NULL) {
        // cleanup
        free(md5);
        archive_read_free(a);
        g_free(desktop_filename);
        g_key_file_free(key_file);

        return -1;
    }

    GError *error = NULL;
    gboolean rv = g_key_file_get_boolean(key_file, G_KEY_FILE_DESKTOP_GROUP, "X-AppImage-Integrate", &error);

    // cleanup
    free(md5);
    archive_read_free(a);
    g_free(desktop_filename);
    g_key_file_free(key_file);

    int result;

    if (!rv) {
        // if the key file hasn't been found and the error is not set to NOT_FOUND, return an error
        if (error != NULL) {
            if (error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                result = 0;
            else
                result = -1;
        }
        else {
            result = 1;
        }
    } else {
        result = 0;
    }

    if (error != NULL)
        g_error_free(error);

    return result;
};

int appimage_type2_shall_not_be_integrated(const char* path) {
    // check if file exists
    if (!g_file_test(path, G_FILE_TEST_IS_REGULAR))
        return -1;

    // check if file is of correct type
    if (appimage_get_type(path, false) != 2)
        return -1;

    char* md5 = appimage_get_md5(path);

    if (md5 == NULL)
        return -1;

    ssize_t fs_offset = appimage_get_elf_size(path);

    if (fs_offset < 0)
        return -1;

    sqfs fs;

    sqfs_err err = sqfs_open_image(&fs, path, (size_t) fs_offset);

    if (err != SQFS_OK) {
        free(md5);
        sqfs_destroy(&fs);
        return -1;
    }

    gchar* desktop_filename = NULL;

    // a structure that will hold the information from the desktop file
    GKeyFile* key_file = g_key_file_new();

    if (!appimage_type2_get_desktop_filename_and_key_file(&fs, &desktop_filename, md5, &key_file, false)) {
        // cleanup
        free(md5);
        free(desktop_filename);
        sqfs_destroy(&fs);
        g_key_file_free(key_file);

        return -1;
    }

    // validate that both have been set to a non-NULL value
    if (desktop_filename == NULL || key_file == NULL) {
        // cleanup
        free(md5);
        sqfs_destroy(&fs);
        g_free(desktop_filename);
        g_key_file_free(key_file);

        return -1;
    }

    // no longer used
    free(md5);

    GError *error = NULL;
    gboolean rv = g_key_file_get_boolean(key_file, G_KEY_FILE_DESKTOP_GROUP, "X-AppImage-Integrate", &error);

    // cleanup
    free(desktop_filename);
    sqfs_destroy(&fs);
    g_key_file_free(key_file);

    int result;

    if (!rv) {
        // if the key file hasn't been found and the error is not set to NOT_FOUND, return an error
        if (error != NULL) {
            if (error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                result = 0;
            else
                result = -1;
        } else {
            result = 1;
        }
    } else {
        result = 0;
    }

    if (error != NULL)
        g_error_free(error);

    return result;
};

/*
 * Checks whether an AppImage's desktop file has set X-AppImage-Integrate=false.
 * Useful to check whether the author of an AppImage doesn't want it to be integrated.
 *
 * Returns >0 if set, 0 if not set, <0 on errors.
 */
int appimage_shall_not_be_integrated(const char *path) {
    // check if file exists
    if (!g_file_test(path, G_FILE_TEST_IS_REGULAR))
        return -1;

    int type = appimage_get_type(path, false);

    switch (type) {
        case 1:
            return appimage_type1_shall_not_be_integrated(path);
        case 2:
            return appimage_type2_shall_not_be_integrated(path);
        default:
            return -1;
    }
}

char* appimage_registered_desktop_file_path(const char *path, char *md5, bool verbose) {
    glob_t pglob = {};

    // if md5 has been calculated before, we can just use it to save these extra calculations
    // if not, we need to calculate it here
    if (md5 == NULL)
        md5 = appimage_get_md5(path);

    // sanity check
    if (md5 == NULL) {
        if (verbose)
            fprintf(stderr, "appimage_get_md5() failed\n");
        return NULL;
    }

    char *data_home = xdg_data_home();

    // TODO: calculate this value exactly
    char *glob_pattern = malloc(PATH_MAX);
    sprintf(glob_pattern, "%s/applications/appimagekit_%s-*.desktop", data_home, md5);

    glob(glob_pattern, 0, NULL, &pglob);

    char* rv = NULL;

    if (pglob.gl_pathc <= 0) {
        if (verbose) {
            fprintf(stderr, "No results found by glob()");
        }
    } else if (pglob.gl_pathc >= 1) {
        if (pglob.gl_pathc > 1 && verbose) {
            fprintf(stderr, "Too many results returned by glob(), returning first result found");
        }

        // need to copy value to be able to globfree() later on
        rv = strdup(pglob.gl_pathv[0]);
    }

    globfree(&pglob);

    return rv;
};

/* Check whether AppImage is registered in the system already */
bool appimage_is_registered_in_system(const char* path) {
    // To check whether an AppImage has been integrated, we just have to check whether the desktop file is in place

    if (!g_file_test(path, G_FILE_TEST_IS_REGULAR))
        return false;

    gchar* md5 = appimage_get_md5(path);

    GKeyFile* key_file = g_key_file_new();
    gchar* desktop_file_path = appimage_registered_desktop_file_path(path, md5, false);

    bool rv = true;

    if (!g_file_test(desktop_file_path, G_FILE_TEST_IS_REGULAR))
        rv = false;

    g_free(md5);
    g_free(desktop_file_path);
    g_key_file_free(key_file);

    return rv;
}

/*
 * Register an AppImage in the system
 * Returns 0 on success, non-0 otherwise.
 */
int appimage_register_in_system(const char* path, bool verbose) {
    if ((g_str_has_suffix(path, ".part")) ||
        g_str_has_suffix(path, ".tmp") ||
        g_str_has_suffix(path, ".download") ||
        g_str_has_suffix(path, ".zs-old") ||
        g_str_has_suffix(path, ".~")
        ) {
        return 1;
    }

    int type = appimage_get_type(path, verbose);
    bool succeed = true;

    if (type != -1) {
#ifdef STANDALONE
        fprintf(stderr, "\n-> Registering type %d AppImage: %s\n", type, path);
#endif
        appimage_create_thumbnail(path, false);

        char* temp_dir = desktop_integration_create_tempdir();
        char* md5 = appimage_get_md5(path);
        char* data_home = xdg_data_home();

        // Files are extracted to a temporary dir to avoid several traversals on the AppImage file
        // Also, they need to be edited by us anyway, and to avoid confusing desktop environments with
        // too many different desktop files, we edit them beforehand and move them into their target
        // destination afterwards only.
        // (Yes, it _could_ probably be done without tempfiles, but given the amount of desktop registrations,
        // we consider the file I/O overhead to be acceptable.)
        desktop_integration_extract_relevant_files(path, temp_dir);
        succeed = succeed && desktop_integration_modify_desktop_file(path, temp_dir, md5);
        succeed = succeed && desktop_integration_move_files_to_user_data_dir(temp_dir, data_home, md5);
        desktop_integration_remove_tempdir(temp_dir);

        free(data_home);
        free(md5);
        free(temp_dir);
    } else {
#ifdef STANDALONE
        fprintf(stderr, "Error: unknown AppImage type %d\n", type);
#endif
        if (verbose)
            fprintf(stderr, "-> Skipping file %s\n", path);
        return 0;
    }

    return succeed ? 0 : 1;
}

/* Delete the thumbnail for a given file and size if it exists */
void delete_thumbnail(char *path, char *size, gboolean verbose)
{
    gchar *thumbnail_path = get_thumbnail_path(path, size, verbose);
    if(verbose)
        fprintf(stderr, "get_thumbnail_path: %s\n", thumbnail_path);
    if(g_file_test(thumbnail_path, G_FILE_TEST_IS_REGULAR)){
        g_unlink(thumbnail_path);
        if(verbose)
            fprintf(stderr, "deleted: %s\n", thumbnail_path);
    }
    g_free(thumbnail_path);
}

/* Recursively delete files in path and subdirectories that contain the given md5
 */
void unregister_using_md5_id(const char *name, int level, char* md5, gboolean verbose)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir)))
        return;

    do {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
            path[len] = 0;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            unregister_using_md5_id(path, level + 1, md5, verbose);
        }

        else {
            gchar *needle = g_strdup_printf("%s_%s", vendorprefix, md5);
            if(strstr(entry->d_name, needle)) {
                    gchar *path_to_be_deleted = g_strdup_printf("%s/%s", name, entry->d_name);
                    if(g_file_test(path_to_be_deleted, G_FILE_TEST_IS_REGULAR)){
                        g_unlink(path_to_be_deleted);
                        if(verbose)
                            fprintf(stderr, "deleted: %s\n", path_to_be_deleted);
                    }
                    g_free(path_to_be_deleted);
                }
            g_free(needle);
        }
    } while ((entry = readdir(dir)) != NULL);
    closedir(dir);
}


/* Unregister an AppImage in the system */
int appimage_unregister_in_system(const char *path, bool verbose)
{
    char *md5 = appimage_get_md5(path);

    /* The file is already gone by now, so we can't determine its type anymore */
#ifdef STANDALONE
    fprintf(stderr, "_________________________\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "-> UNREGISTER %s\n", path);
#endif
    /* Could use gnome_desktop_thumbnail_factory_lookup instead of the next line */

    /* Delete the thumbnails if they exist */
    delete_thumbnail(path, "normal", verbose); // 128x128
    delete_thumbnail(path, "large", verbose); // 256x256

    char* data_home = xdg_data_home();
    unregister_using_md5_id(data_home, 0, md5, verbose);
    g_free(data_home);

    g_free(md5);

    return 0;
}

bool move_file(const char* source, const char* target) {
    g_type_init();
    bool succeed = true;
    GError *error = NULL;
    GFile *icon_file = g_file_new_for_path(source);
    GFile *target_file = g_file_new_for_path(target);
    if (!g_file_move (icon_file, target_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error)) {
#ifdef STANDALONE
        fprintf(stderr, "Error moving file: %s\n", error->message);
#endif
        succeed = false;
        g_clear_error (&error);
    }

    g_object_unref(icon_file);
    g_object_unref(target_file);

    return succeed;
}

struct extract_appimage_file_command_data {
    const char *path;
    const char *destination;
    char *link;
};
struct read_appimage_file_into_buffer_command_data {
    char* file_path;
    char* out_buffer;
    char* link_path;
    unsigned long out_buf_size;
    bool success;
};

void extract_appimage_file_command(void* handler_data, void* entry_data, void* user_data) {
    appimage_handler* h = handler_data;
    struct extract_appimage_file_command_data * params = user_data;

    char* filename = h->get_file_name(h, entry_data);
    if (strcmp(params->path, filename) == 0) {
        params->link = h->get_file_link(handler_data, entry_data);

        h->extract_file(h, entry_data, params->destination);
    }


    free(filename);
}

void read_appimage_file_into_buffer_command(void* handler_data, void* entry_data, void* user_data) {
    appimage_handler* h = handler_data;
    struct read_appimage_file_into_buffer_command_data* params = user_data;

    if (h->read_file_into_new_buffer == NULL) {
#ifdef STANDALONE
        fprintf(stderr, "read_file_into_new_buffer is NULL, go fix that!\n");
#endif
        return;
    }

    char* filename = h->get_file_name(h, entry_data);
    if (strcmp(params->file_path, filename) == 0) {
        params->link_path = h->get_file_link(h, entry_data);
        params->success = h->read_file_into_new_buffer(h, entry_data, &params->out_buffer, &(params->out_buf_size));
    }


    free(filename);
}

void extract_appimage_icon_command(void *handler_data, void *entry_data, void *user_data) {
    appimage_handler *h = handler_data;
    struct archive_entry *entry = entry_data;
    gchar *path = user_data;

    char *filename = h->get_file_name(h, entry);
    if (strcmp(".DirIcon", filename) == 0)
        h->extract_file(h, entry, path);

    free(filename);
}

void extract_appimage_icon(appimage_handler *h, gchar *target) {
    h->traverse(h, extract_appimage_icon_command, target);
}

/* Create AppImage thumbanil according to
 * https://specifications.freedesktop.org/thumbnail-spec/0.8.0/index.html
 */
void appimage_create_thumbnail(const char *appimage_file_path, bool verbose) {
    // extract AppImage icon to /tmp
    appimage_handler handler = create_appimage_handler(appimage_file_path);

    char *tmp_path = "/tmp/appimage_thumbnail_tmp";
    extract_appimage_icon(&handler, tmp_path);

    if (g_file_test(tmp_path, G_FILE_TEST_EXISTS) ) {
        // TODO: transform it to png with sizes 128x128 and 254x254
        gchar *target_path = get_thumbnail_path(appimage_file_path, "normal", verbose);

        mk_base_dir(target_path);

        // deploy icon as thumbnail
        move_file (tmp_path, target_path);

        // clean up
        g_free(target_path);
    } else {
#ifdef STANDALONE
        fprintf(stderr, "ERROR: Icon file not extracted: %s", tmp_path);
#endif
    }

}

void appimage_extract_file_following_symlinks(const gchar* appimage_file_path, const char* file_path,
                                              const char* target_file_path) {

    struct extract_appimage_file_command_data data;
    data.link = strdup(file_path);
    data.destination = target_file_path;

    bool looping = false;
    GSList* visited_entries = NULL;

    do {
        visited_entries = g_slist_prepend(visited_entries, data.link);
        data.path = data.link;
        data.link = NULL;

        appimage_handler handler = create_appimage_handler(appimage_file_path);
        handler.traverse(&handler, extract_appimage_file_command, &data);

        if (data.link) {
            if (data.link && visited_entries &&
                g_slist_find_custom(visited_entries, data.link, (GCompareFunc) strcmp))
                looping = true;

            g_remove(target_file_path);
        }

    } while (data.link && !looping);

    if (visited_entries)
        g_slist_free_full(visited_entries, free);
}

bool appimage_read_file_into_buffer_following_symlinks(const char* appimage_file_path, const char* file_path,
                                                       char** buffer, unsigned long* buf_size) {

    struct read_appimage_file_into_buffer_command_data data;
    data.link_path = strdup(file_path);

    data.out_buffer = NULL;
    GSList *visited_entries = NULL;

    do {
        visited_entries = g_slist_prepend(visited_entries, data.link_path);

        // prepare an empty struct
        data.file_path = data.link_path;
        data.link_path = NULL;

        // release any data that could be allocated in previous iterations
        if (data.out_buffer ) {
            free(data.out_buffer);
            data.out_buffer = NULL;
            data.out_buf_size = 0;
        }

        data.success = false;

        appimage_handler handler = create_appimage_handler(appimage_file_path);
        handler.traverse(&handler, &read_appimage_file_into_buffer_command, &data);

        // Find loops
        if (data.link_path && visited_entries &&
            g_slist_find_custom(visited_entries, data.link_path, (GCompareFunc) strcmp))
                data.success = false;
    } while (data.success && data.link_path != NULL);

    if (visited_entries)
        g_slist_free_full(visited_entries, free);

    if (!data.success) {
        free(data.out_buffer);

        *buffer = NULL;
        *buf_size = 0;
    } else {
        *buffer = data.out_buffer;
        *buf_size = data.out_buf_size;
    }

    return data.success;
}

void extract_appimage_file_name(void *handler_data, void *entry_data, void *user_data) {
    appimage_handler *h = handler_data;
    struct archive_entry *entry = entry_data;
    GList **list = user_data;

    char *filename = h->get_file_name(h, entry);

    GList* ptr = g_list_find_custom (*list, filename, g_strcmp0);

    if (ptr == NULL)
        *list = g_list_append(*list, filename);
    else
        free(filename);
}


char** appimage_list_files(const char *path) {
    GList *list = NULL;
    appimage_handler handler = create_appimage_handler(path);

    handler.traverse(&handler, extract_appimage_file_name, &list);

    int n = g_list_length(list);
    char **result = malloc(sizeof(char*) * (n+1) );
    result[n] = NULL;

    GList *itr = list;
    for (int i = 0; i < n; i ++) {
        result[i] = (char *) itr->data;
        itr = itr->next;
    }


    g_list_free(list);

    return result;
}

void appimage_string_list_free(char** list) {
    for (char **ptr = list; ptr != NULL && *ptr != NULL; ptr ++)
        free(*ptr);

    free(list);
}


/* Check if a file is an AppImage. Returns the image type if it is, or -1 if it isn't */
int appimage_get_type(const char* path, bool verbose)
{
    FILE *f = fopen(path, "rt");
    if (f != NULL)
    {
        char buffer[3] = {0};

        /* Check magic bytes at offset 8 */
        fseek(f, 8, SEEK_SET);
        fread(buffer, 1, 3, f);
        fclose(f);
        if(match_type_1_magic_bytes(buffer)){
#ifdef STANDALONE
            fprintf(stderr, "_________________________\n");
#endif
            if(verbose){
                fprintf(stderr, "AppImage type 1\n");
            }
            return 1;
        } else if((buffer[0] == 0x41) && (buffer[1] == 0x49) && (buffer[2] == 0x02)){
#ifdef STANDALONE
            fprintf(stderr, "_________________________\n");
#endif
            if(verbose){
                fprintf(stderr, "AppImage type 2\n");
            }
            return 2;
        } else {
            if (is_iso_9660_file(path) && (appimage_get_elf_size(path) != -1)) {
#ifdef STANDALONE
                fprintf(stderr, "_________________________\n");
#endif
                if (verbose) {
                    fprintf(stderr, "This file seems to be an AppImage type 1 without magic bytes\n");
                    fprintf(stderr, "The AppImage author should embed the magic bytes,"
                                    " see https://github.com/AppImage/AppImageSpec\n");
                }
                return 1;
            } else {
#ifdef STANDALONE
                fprintf(stderr, "_________________________\n");
#endif
                if(verbose){
                    fprintf(stderr, "Unrecognized file '%s'\n", path);
                }
                return -1;
            }
        }
    }
    return -1;
}
