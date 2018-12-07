#include <cstring>
#include <iostream>


extern "C" {
#include <sys/stat.h>

#include <nonstd.h>
#include <squashfuse.h>
#include <squashfs_fs.h>
}

#include <boost/filesystem.hpp>

#include <appimage/appimage.h>
#include "AppImageErrors.h"
#include "AppImageType2Traversal.h"
#include "AppImageDummyStreamBuffer.h"

using namespace std;

AppImage::AppImageType2Traversal::AppImageType2Traversal(std::string path) : path(path) {
    cout << "Opening " << path << " as Type 2 AppImage" << endl;
    // The offset at which a squashfs image is expected
    ssize_t fs_offset = appimage_get_elf_size(path.c_str());

    if (fs_offset < 0)
        throw AppImageReadError("get_elf_size error");

    sqfs_err err = sqfs_open_image(&fs, path.c_str(), (size_t) fs_offset);

    if (err != SQFS_OK)
        throw AppImageReadError("sqfs_open_image error: " + path);

    root_inode = sqfs_inode_root(&fs);
    err = sqfs_traverse_open(&trv, &fs, root_inode);
    if (err != SQFS_OK)
        throw AppImageReadError("sqfs_traverse_open error");

}

AppImage::AppImageType2Traversal::~AppImageType2Traversal() {
    sqfs_traverse_close(&trv);

    cout << "Closing " << path << " as Type 2 AppImage" << endl;
    sqfs_destroy(&fs);
}

void AppImage::AppImageType2Traversal::next() {
    sqfs_err err;
    if (!sqfs_traverse_next(&trv, &err))
        completed = true;

    if (err != SQFS_OK)
        throw AppImageReadError("sqfs_traverse_next error");
}

bool AppImage::AppImageType2Traversal::isCompleted() {
    return completed;
}

std::string AppImage::AppImageType2Traversal::getEntryName() {
    if (trv.path != nullptr)
        return trv.path;
    else
        return string();
}

void AppImage::AppImageType2Traversal::extract(const std::string& target) {
    sqfs_inode inode;
    if (sqfs_inode_get(&fs, &inode, trv.entry.inode))
        throw AppImageReadError("sqfs_inode_get error");

    boost::filesystem::path targetPath(target);
    boost::filesystem::create_directories(targetPath.parent_path());

    switch (inode.base.inode_type) {
        case SQUASHFS_DIR_TYPE:
        case SQUASHFS_LDIR_TYPE:
            extractDir(target);
            break;
        case SQUASHFS_REG_TYPE:
        case SQUASHFS_LREG_TYPE:
            extractFile(inode, target);
            break;
        case SQUASHFS_SYMLINK_TYPE:
        case SQUASHFS_LSYMLINK_TYPE:
            extractSymlink(inode, target);
            break;
        default:
            throw AppImageError("AppImage Type 2 inode.base.inode_type " +
                                std::to_string(inode.base.inode_type) + " not supported yet.");
    }
}

sqfs_err AppImage::AppImageType2Traversal::sqfs_stat(sqfs* fs, sqfs_inode* inode, struct stat* st) {
    sqfs_err err = SQFS_OK;
    uid_t id;

    memset(st, 0, sizeof(*st));
    st->st_mode = inode->base.mode;
    st->st_nlink = inode->nlink;
    st->st_mtime = st->st_ctime = st->st_atime = inode->base.mtime;

    if (S_ISREG(st->st_mode)) {
        /* FIXME: do symlinks, dirs, etc have a size? */
        st->st_size = inode->xtra.reg.file_size;
        st->st_blocks = st->st_size / 512;
    } else if (S_ISBLK(st->st_mode) || S_ISCHR(st->st_mode)) {
        st->st_rdev = sqfs_makedev(inode->xtra.dev.major,
                                   inode->xtra.dev.minor);
    } else if (S_ISLNK(st->st_mode)) {
        st->st_size = inode->xtra.symlink_size;
    }

    st->st_blksize = fs->sb.block_size; /* seriously? */

    err = sqfs_id_get(fs, inode->base.uid, &id);
    if (err)
        return err;
    st->st_uid = id;
    err = sqfs_id_get(fs, inode->base.guid, &id);
    st->st_gid = id;
    if (err)
        return err;

    return SQFS_OK;
}

void AppImage::AppImageType2Traversal::extractDir(const std::string& target) {
    if (access(target.c_str(), F_OK) == -1) {
        if (mkdir(target.c_str(), 0777) == -1)
            throw AppImageError("mkdir error at " + target);
    }
}

void AppImage::AppImageType2Traversal::extractFile(sqfs_inode inode, const std::string& target) {
    struct stat st;
    if (sqfs_stat(&fs, &inode, &st) != 0)
        throw AppImageReadError("sqfs_stat error");

    // Read the file in chunks
    off_t bytes_already_read = 0;
    sqfs_off_t bytes_at_a_time = 64 * 1024;
    FILE* f;
    f = fopen(target.c_str(), "w+");
    if (f == NULL)
        throw AppImageError("fopen error on " + target);
    while (bytes_already_read < inode.xtra.reg.file_size) {
        char buf[bytes_at_a_time];
        if (sqfs_read_range(&fs, &inode, (sqfs_off_t) bytes_already_read, &bytes_at_a_time, buf))
            throw AppImageReadError("sqfs_read_range error");
        // fwrite(buf, 1, bytes_at_a_time, stdout);
        fwrite(buf, 1, bytes_at_a_time, f);
        bytes_already_read = bytes_already_read + bytes_at_a_time;
    }
    fclose(f);
    chmod(target.c_str(), st.st_mode);
}

void AppImage::AppImageType2Traversal::extractSymlink(sqfs_inode inode, const std::string& target) {
    size_t size = strlen(trv.path) + 1;
    char buf[size];
    int ret = sqfs_readlink(&fs, &inode, buf, &size);
    if (ret != 0)
        throw AppImageReadError("sqfs_readlink error");
    unlink(target.c_str());
    ret = symlink(buf, target.c_str());
    if (ret != 0)
        throw AppImageReadError("symlink error at " + target);
}

istream& AppImage::AppImageType2Traversal::read() {
    auto dummyStreamBuffer = new AppImageDummyStreamBuffer();
    auto istream = new std::istream(dummyStreamBuffer);

    return *istream;
}
