// library includes
#include <appimage/appimage.h>
#include <squashfuse.h>
#include <squashfs_fs.h>

// local includes
#include "type2.h"

void appimage_type2_open(appimage_handler* handler) {
    if (is_handler_valid(handler) && !handler->is_open) {
#ifdef STANDALONE
        fprintf(stderr, "Opening %s as Type 2 AppImage\n", handler->path);
#endif
        // The offset at which a squashfs image is expected
        ssize_t fs_offset = appimage_get_elf_size(handler->path);

        if (fs_offset < 0) {
#ifdef STANDALONE
            fprintf(stderr, "get_elf_size error\n");
#endif
            handler->is_open = false;
            handler->cache = NULL;
            return;
        }

        sqfs* fs = malloc(sizeof(sqfs));
        sqfs_err err = sqfs_open_image(fs, handler->path, (size_t) fs_offset);
        if (err != SQFS_OK) {
#ifdef STANDALONE
            fprintf(stderr, "sqfs_open_image error: %s\n", handler->path);
#endif
            free(fs);
            handler->is_open = false;
            handler->cache = NULL;
        } else {
            handler->is_open = true;
            handler->cache = fs;
        }
    }
}

void appimage_type2_close(appimage_handler* handler) {
    if (is_handler_valid(handler) && handler->is_open) {
#ifdef STANDALONE
        fprintf(stderr, "Closing %s\n", handler->path);
#endif

        sqfs_destroy(handler->cache);
        free(handler->cache);

        handler->is_open = false;
        handler->cache = NULL;
    }
}

// forward declaration, see below
void appimage_type2_extract_symlink(sqfs* fs, sqfs_inode* inode, const char* target);
// TODO: get rid of this forward declaration
void squash_extract_inode_to_file(sqfs *fs, sqfs_inode *inode, const gchar *dest);

void appimage_type2_extract_regular_file(sqfs* fs, sqfs_inode* inode, const char* target) {
    mk_base_dir(target);

    // Read the file in chunks
    squash_extract_inode_to_file(fs, inode, target);
}

bool appimage_type2_resolve_symlink(sqfs* fs, sqfs_inode* inode) {
    // no need to do anything if the passed inode is not a symlink
    if (inode->base.inode_type != SQUASHFS_SYMLINK_TYPE)
        return true;

    // read twice: once to populate size to be able to allocate the right amount of memory, then to populate the buffer
    size_t size;
    sqfs_readlink(fs, inode, NULL, &size);

    char buf[size];
    int ret = sqfs_readlink(fs, inode, buf, &size);

    if (ret != 0) {
#ifdef STANDALONE
        fprintf(stderr, "WARNING: Symlink error.");
#endif
        return false;
    }

    sqfs_err err = sqfs_inode_get(fs, inode, fs->sb.root_inode);
    if (err != SQFS_OK) {
#ifdef STANDALONE
        fprintf(stderr, "WARNING: Unable to get the root inode. Error: %d", err);
#endif
        return false;
    }

    bool found = false;
    err = sqfs_lookup_path(fs, inode, buf, &found);
    if (err != SQFS_OK) {
#ifdef STANDALONE
        fprintf(stderr, "WARNING: There was an error while trying to lookup a symblink. Error: %d", err);
#endif
        return false;
    }

    return true;
}

bool appimage_type2_extract_file_following_symlinks(sqfs* fs, sqfs_inode* inode, const char* target) {
    if (!appimage_type2_resolve_symlink(fs, inode)) {
#ifdef STANDALONE
        fprintf(stderr, "ERROR: Failed to resolve symlink");
#endif
        return false;
    }

    if (inode->base.inode_type != SQUASHFS_REG_TYPE) {
#ifdef STANDALONE
        fprintf(stderr, "WARNING: Unable to extract file of type %d", inode->base.inode_type);
#endif
        return false;
    }

    appimage_type2_extract_regular_file(fs, inode, target);
    return true;
}

void type2_traverse(appimage_handler* handler, traverse_cb command, void* command_data) {
    appimage_type2_open(handler);

    if (handler->is_open && handler->cache != NULL) {
        sqfs* fs = handler->cache;
        sqfs_traverse trv;
        sqfs_inode_id root_inode = sqfs_inode_root(fs);
        sqfs_err err = sqfs_traverse_open(&trv, fs, root_inode);
        if (err != SQFS_OK) {
#ifdef STANDALONE
            fprintf(stderr, "sqfs_traverse_open error\n");
#endif
        }
        while (sqfs_traverse_next(&trv, &err))
            command(handler, &trv, command_data);

        if (err) {
#ifdef STANDALONE
            fprintf(stderr, "sqfs_traverse_next error\n");
#endif
        }
        sqfs_traverse_close(&trv);
    }

    appimage_type2_close(handler);
}

char* type2_get_file_name(appimage_handler* handler, void* data) {
    (void) handler;
    sqfs_traverse* trv = data;
    return strdup(trv->path);
}

void type2_extract_file(appimage_handler* handler, void* data, const char* target) {
    sqfs* fs = handler->cache;
    sqfs_traverse* trv = data;

    sqfs_inode inode;
    if (sqfs_inode_get(fs, &inode, trv->entry.inode)) {
#ifdef STANDALONE
        fprintf(stderr, "sqfs_inode_get error\n");
#endif
    }

    appimage_type2_extract_file_following_symlinks(fs, &inode, target);
}

bool type2_read_file_into_buf(struct appimage_handler* handler, void* traverse, char** buffer, unsigned long* buf_size) {
    sqfs* fs = handler->cache;
    sqfs_traverse* trv = traverse;

    sqfs_inode inode;
    if (sqfs_inode_get(fs, &inode, trv->entry.inode)) {
#ifdef STANDALONE
        fprintf(stderr, "sqfs_inode_get error\n");
#endif
    }

    // resolve symlink if possible
    if (!appimage_type2_resolve_symlink(fs, &inode)) {
#ifdef STANDALONE
        fprintf(stderr, "ERROR: Failed to resolve symlink");
#endif
        return false;
    }

    if (inode.base.inode_type != SQUASHFS_REG_TYPE) {
#ifdef STANDALONE
        fprintf(stderr, "WARNING: Unable to extract file of type %d", inode->base.inode_type);
#endif
        return false;
    }

    uint64_t file_size = inode.xtra.reg.file_size;

    char* new_buffer = (char*) malloc(sizeof(char) * file_size);

    if (new_buffer == NULL) {
#ifdef STANDALONE
        fprintf(stderr, "failed to allocate enough memory for buffer (required: %ul bytes)\n", file_size);
#endif
        return false;
    }

    if (sqfs_read_range(fs, &inode, 0, (sqfs_off_t*) &file_size, new_buffer) != SQFS_OK) {
#ifdef STANDALONE
        fprintf(stderr, "failed to read data into buffer\n");
#endif
        free(new_buffer);
        return false;
    }

    *buffer = new_buffer;
    *buf_size = file_size;
    return true;
}

char* type2_get_file_link(struct appimage_handler* handler, void* data) {
    sqfs_traverse* trv = data;

    sqfs_inode inode;
    if (!sqfs_inode_get(trv->fs, &inode, trv->entry.inode))
        return NULL;

    // read twice: once to populate size to be able to allocate the right amount of memory, then to populate the buffer
    size_t size;
    sqfs_readlink(trv->fs, &inode, NULL, &size);

    char* buf = malloc(sizeof(char) * size);
    int ret = sqfs_readlink(trv->fs, &inode, buf, &size);

    return buf;
}

appimage_handler appimage_type_2_create_handler() {
    appimage_handler h;
    h.traverse = type2_traverse;
    h.get_file_name = type2_get_file_name;
    h.get_file_link = type2_get_file_link;
    h.extract_file = type2_extract_file;
    h.read_file_into_new_buffer = type2_read_file_into_buf;
    h.type = 2;

    return h;
}
