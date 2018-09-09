// system includes
#include <fcntl.h>

// library includes
#include <archive.h>
#include <archive_entry.h>

// local includes
#include "type2.h"

void appimage_type1_open(appimage_handler* handler) {
    if (is_handler_valid(handler) && !handler->is_open) {
#ifdef STANDALONE
        fprintf(stderr, "Opening %s as Type 1 AppImage\n", handler->path);
#endif
        struct archive* a;
        a = archive_read_new();
        archive_read_support_format_iso9660(a);
        if (archive_read_open_filename(a, handler->path, 10240) != ARCHIVE_OK) {
            fprintf(stderr, "%s", archive_error_string(a));
            handler->cache = NULL;
            handler->is_open = false;
        } else {
            handler->cache = a;
            handler->is_open = true;
        }
    }
}

void appimage_type1_close(appimage_handler* handler) {
    if (is_handler_valid(handler) && handler->is_open) {
#ifdef STANDALONE
        fprintf(stderr, "Closing %s\n", handler->path);
#endif
        struct archive* a = handler->cache;
        archive_read_close(a);
        archive_read_free(a);

        handler->cache = NULL;
        handler->is_open = false;
    }
}

void type1_traverse(appimage_handler* handler, traverse_cb command, void* command_data) {
    appimage_type1_open(handler);

    if (!command) {
#ifdef STANDALONE
        fprintf(stderr, "No traverse command set.\n");
#endif
        return;
    }

    if (handler->is_open) {
        struct archive* a = handler->cache;
        struct archive_entry* entry;
        int r;

        for (;;) {
            r = archive_read_next_header(a, &entry);
            if (r == ARCHIVE_EOF) {
                break;
            }
            if (r != ARCHIVE_OK) {
                fprintf(stderr, "%s\n", archive_error_string(a));
                break;
            }

            /* Skip all but regular files; FIXME: Also handle symlinks correctly */
            if (archive_entry_filetype(entry) != AE_IFREG) {
                continue;
            }

            command(handler, entry, command_data);
        }
    }

    appimage_type1_close(handler);
}

// TODO: remove forward declaration
gchar* replace_str(const gchar* src, const gchar* find, const gchar* replace);

char* type1_get_file_name(appimage_handler* handler, void* data) {
    (void) handler;

    struct archive_entry* entry = (struct archive_entry*) data;

    char* filename = replace_str(archive_entry_pathname(entry), "./", "");
    return filename;
}

void type1_extract_file(appimage_handler* handler, void* data, const char* target) {
    (void) data;

    struct archive* a = handler->cache;
    mk_base_dir(target);

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int f = open(target, O_WRONLY | O_CREAT | O_TRUNC, mode);

    if (f == -1){
#ifdef STANDALONE
        fprintf(stderr, "open error: %s\n", target);
#endif
        return;
    }

    archive_read_data_into_fd(a, f);
    close(f);
}

appimage_handler appimage_type_1_create_handler() {
    appimage_handler h;
    h.traverse = type1_traverse;
    h.get_file_name = type1_get_file_name;
    h.extract_file = type1_extract_file;
    // TODO
    h.read_file_into_new_buffer = NULL;
    h.type = 1;

    return h;
}
