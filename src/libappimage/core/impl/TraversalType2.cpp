// system
#include <cstring>
#include <iostream>
#include <fstream>
#include <set>
#include <fcntl.h>
extern "C" {
#include <unistd.h>
#include <sys/stat.h>
}

// libraries
#include <boost/filesystem.hpp>
extern "C" {
#include <nonstd.h>
#include <squashfuse.h>
#include <squashfs_fs.h>
}

// local
#include "appimage/core/AppImage.h"
#include "appimage/core/exceptions.h"
#include "PayloadIStream.h"
#include "StreambufType2.h"
#include "TraversalType2.h"

using namespace std;
using namespace appimage::core::impl;

TraversalType2::TraversalType2(std::string path) : path(path) {
    clog << "Opening " << path << " as Type 2 AppImage" << endl;
    // read the offset at which a squashfs image is expected to start
    ssize_t fs_offset = core::AppImage(path).getPayloadOffset();

    if (fs_offset < 0)
        throw IOError("get_elf_size error");

    sqfs_err err = sqfs_open_image(&fs, path.c_str(), (size_t) fs_offset);

    if (err != SQFS_OK)
        throw IOError("sqfs_open_image error: " + path);

    // prepare for traverse
    rootInodeId = sqfs_inode_root(&fs);
    err = sqfs_traverse_open(&trv, &fs, rootInodeId);
    if (err != SQFS_OK) {
        sqfs_destroy(&fs);
        throw IOError("sqfs_traverse_open error");
    }

    // Fetch first entry
    next();
}

TraversalType2::~TraversalType2() {
    sqfs_traverse_close(&trv);

    clog << "Closing " << path << " as Type 2 AppImage" << endl;
    sqfs_destroy(&fs);
}

void TraversalType2::next() {
    sqfs_err err;
    if (!sqfs_traverse_next(&trv, &err))
        completed = true;

    if (err != SQFS_OK)
        throw IOError("sqfs_traverse_next error");

    if (!completed) {
        currentEntryType = readEntryType();
        currentEntryPath = readEntryName();
        currentEntryLink = currentEntryType == PayloadEntryType::LINK ? readEntryLink() : std::string();
    } else {
        currentEntryType = PayloadEntryType::UNKNOWN;
        currentEntryPath = std::string();
        currentEntryLink = std::string();
    }
}

bool TraversalType2::isCompleted() const {
    return completed;
}

std::string TraversalType2::getEntryName() const {
    return currentEntryPath;
}

appimage::core::PayloadEntryType TraversalType2::getEntryType() const {
    return currentEntryType;
}

void TraversalType2::extract(const std::string& target) {
    sqfs_inode inode;
    if (sqfs_inode_get(&fs, &inode, trv.entry.inode))
        throw IOError("sqfs_inode_get error");

    // create target parent dir
    auto parentPath = boost::filesystem::path(target).parent_path();
    boost::filesystem::create_directories(parentPath);

    // handle each inode type properly
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
                                std::to_string(inode.base.inode_type) + " not supported yet");
    }
}

sqfs_err TraversalType2::sqfs_stat(sqfs* fs, sqfs_inode* inode, struct stat* st) {
    // code borrowed from the "extract.c" file at squashfuse
    sqfs_err err;
    uid_t id;

    memset(st, 0, sizeof(*st));
    // fill stats
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

void TraversalType2::extractDir(const std::string& target) {
    // The directory doesn't exists
    if (access(target.c_str(), F_OK) == -1) {
        // Create new directory with 755 permissions
        if (mkdir(target.c_str(), 0755) == -1)
            throw FileSystemError("mkdir error at " + target);
    }
}

void TraversalType2::extractFile(sqfs_inode inode, const std::string& target) {
    // Read inode stats
    struct stat st = {};
    if (sqfs_stat(&fs, &inode, &st) != 0)
        throw IOError("sqfs_stat error");

    // open read stream
    auto& istream = read();

    // open write stream
    ofstream targetFile(target);

    // transfer data
    targetFile << istream.rdbuf();
    targetFile.close();

    // set file stats
    chmod(target.c_str(), st.st_mode);
}

void TraversalType2::extractSymlink(sqfs_inode inode, const std::string& target) {
    // read the target link path size
    size_t size;
    sqfs_readlink(&fs, &inode, nullptr, &size);

    std::vector<char> buf(size);

    // read the target link in buf
    int ret = sqfs_readlink(&fs, &inode, buf.data(), &size);
    if (ret != 0)
        throw IOError("sqfs_readlink error");

    // remove any existent link at the target path
    ret = unlink(target.c_str());
    if (ret != 0 && errno != ENOENT)
        throw IOError("unlink error at " + target);

    ret = symlink(buf.data(), target.c_str());
    if (ret != 0)
        throw IOError("symlink error at " + target);
}

istream& TraversalType2::read() {
    // get current inode
    sqfs_inode inode;
    if (sqfs_inode_get(&fs, &inode, trv.entry.inode))
        throw IOError("sqfs_inode_get error");

    // resolve symlinks if any
    if (!resolve_symlink(&inode))
        throw IOError("symlink resolution error");

    // create a streambuf for reading the inode contents
    auto tmpBuffer = new StreambufType2(fs, inode, 1024);

    // replace buffer of the istream
    entryIStream.rdbuf(tmpBuffer);

    // replace and drop old buffer
    entryStreamBuf.reset(tmpBuffer);

    return entryIStream;
}

bool TraversalType2::resolve_symlink(sqfs_inode* inode) {
    sqfs_err err;
    bool found = false;

    sqfs_inode rootInode;
    err = sqfs_inode_get(&fs, &rootInode, rootInodeId);
    if (err != SQFS_OK)
        return false;

    // Save visited inode numbers to prevent a infinite loop in case of cycles between symlinks.
    // A cycle may occur when by example: a (file) -> (links to) b and b -> c and c -> a
    std::set<__le32> inodes_visited;
    inodes_visited.insert(inode->base.inode_number);

    while (inode->base.inode_type == SQUASHFS_SYMLINK_TYPE || inode->base.inode_type == SQUASHFS_LSYMLINK_TYPE) {
        // Read symlink
        size_t size;
        // read twice, once to find out right amount of memory to allocate
        err = sqfs_readlink(&fs, inode, NULL, &size);
        if (err != SQFS_OK)
            return false;

        char symlink_target_path[size];
        // then to populate the buffer
        err = sqfs_readlink(&fs, inode, symlink_target_path, &size);
        if (err != SQFS_OK)
            return false;

        // lookup symlink target path
        *inode = rootInode;
        err = sqfs_lookup_path(&fs, inode, symlink_target_path, &found);

        if (!found)
            return false;

        if (err != SQFS_OK)
            return false;

        // check if we fell into a symlinks cycle
        auto ret = inodes_visited.insert(inode->base.inode_number);
        if (!ret.second) {
            std::clog << "Symlinks loop found ";
            return false;
        }
    }

    return true;
}

string TraversalType2::getEntryLink() const {
    return currentEntryLink;
}

appimage::core::PayloadEntryType TraversalType2::readEntryType() const {
    if (trv.dir_end)
        return PayloadEntryType::DIR;

    switch (trv.entry.type) {
        case SQUASHFS_REG_TYPE:
        case SQUASHFS_LREG_TYPE:
            return PayloadEntryType::REGULAR;

        case SQUASHFS_SYMLINK_TYPE:
        case SQUASHFS_LSYMLINK_TYPE:
            return PayloadEntryType::LINK;

        case SQUASHFS_DIR_TYPE:
        case SQUASHFS_LDIR_TYPE:
            return PayloadEntryType::DIR;

        default:
            return PayloadEntryType::UNKNOWN;
    }
}

std::string TraversalType2::readEntryName() const {
    if (trv.path != nullptr)
        return trv.path;
    else
        return string();
}

std::string TraversalType2::readEntryLink() {
    // get current inode
    sqfs_inode inode;
    if (sqfs_inode_get(&fs, &inode, trv.entry.inode))
        throw IOError("sqfs_inode_get error");

    // read the target link path size
    size_t size;
    auto err = sqfs_readlink(&fs, &inode, nullptr, &size);
    if (err != SQFS_OK)
        throw IOError("sqfs_readlink error");

    std::vector<char> buf(size);

    // read the target link in buf
    err = sqfs_readlink(&fs, &inode, buf.data(), &size);
    if (err != SQFS_OK)
        throw IOError("sqfs_readlink error");


    return buf.data();
}
