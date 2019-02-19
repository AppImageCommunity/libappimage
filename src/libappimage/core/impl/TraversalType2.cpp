// squashfuse includes must go on top to avoid redefinition warning on _POSIX_C_SOURCE
extern "C" {
#include <squashfuse.h>
#include <squashfs_fs.h>

#include <sys/stat.h>
}

// libraries
#include <boost/filesystem.hpp>

// system
#include <cstring>
#include <iostream>
#include <fstream>
#include <set>

// local
#include "appimage/core/AppImage.h"
#include "appimage/core/exceptions.h"
#include "PayloadIStream.h"
#include "StreambufType2.h"
#include "TraversalType2.h"

using namespace std;
namespace bf = boost::filesystem;

namespace appimage {
    namespace core {
        namespace impl {
            class TraversalType2::Priv {
            public:
                explicit Priv(const std::string& path) {
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
                }

                virtual ~Priv() {
                    sqfs_traverse_close(&trv);

                    sqfs_destroy(&fs);
                }

                bool isCompleted() const {
                    return completed;
                }

                PayloadEntryType getCurrentEntryType() const {
                    return currentEntryType;
                }

                const string& getCurrentEntryPath() const {
                    return currentEntryPath;
                }

                const string& getCurrentEntryLink() const {
                    return currentEntryLink;
                }

                void next() {
                    sqfs_err err;
                    if (!sqfs_traverse_next(&trv, &err))
                        completed = true;

                    if (err != SQFS_OK)
                        throw IOError("sqfs_traverse_next error");

                    if (!completed) {
                        currentInode = readInode();
                        currentEntryType = readEntryType();
                        currentEntryPath = readEntryName();
                        currentEntryLink = currentEntryType == PayloadEntryType::LINK ? readEntryLink() : std::string();
                    } else {
                        currentEntryType = PayloadEntryType::UNKNOWN;
                        currentEntryPath = std::string();
                        currentEntryLink = std::string();
                    }
                }

                void extract(const std::string& target) {
                    sqfs_inode inode;
                    if (sqfs_inode_get(&fs, &inode, trv.entry.inode))
                        throw IOError("sqfs_inode_get error");

                    // create target parent dir
                    auto parentPath = bf::path(target).parent_path();
                    bf::create_directories(parentPath);

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

                istream& read() {
                    // resolve symlinks if any
                    if (!resolveSymlink(&currentInode))
                        throw IOError("symlink resolution error");

                    // create a streambuf for reading the inode contents
                    auto tmpBuffer = new StreambufType2(&fs, &currentInode, 1024);

                    // replace buffer of the istream
                    entryIStream.rdbuf(tmpBuffer);

                    // replace and drop old buffer
                    entryStreamBuf.reset(tmpBuffer);

                    return entryIStream;
                }

            private:
                std::string path;
                bool completed = false;

                // squash fuse context
                struct sqfs fs = {};
                sqfs_traverse trv = {};
                sqfs_inode_id rootInodeId = 0;
                sqfs_inode currentInode = {};

                // Current entry data cache
                PayloadEntryType currentEntryType = PayloadEntryType::UNKNOWN;
                std::string currentEntryPath;
                std::string currentEntryLink;

                PayloadIStream entryIStream;
                std::unique_ptr<StreambufType2> entryStreamBuf;

                sqfs_inode readInode() {
                    sqfs_inode inode;
                    if (sqfs_inode_get(&fs, &inode, trv.entry.inode))
                        throw IOError("sqfs_inode_get error");

                    return inode;
                }

                /**
                 * If the <inode> points to a symlink it is followed until a regular file is found.
                 * This method is aware of symlink loops and will fail properly in such case.
                 * @param inode [RETURN PARAMETER] will be filled with a regular file inode. It cannot be NULL
                 * @return succeed true if the file is found, otherwise false
                 * */
                bool resolveSymlink(sqfs_inode* inode) {
                    sqfs_err err;
                    bool found = false;

                    sqfs_inode rootInode;
                    err = sqfs_inode_get(&fs, &rootInode, rootInodeId);
                    if (err != SQFS_OK)
                        return false;

                    // Save visited inode numbers to prevent a infinite loop in case of cycles between symlinks.
                    // A cycle may occur when by example: a (file) -> (links to) b and b -> c and c -> a
                    std::set<__le32> inodesVisited;
                    inodesVisited.insert(inode->base.inode_number);

                    while (inode->base.inode_type == SQUASHFS_SYMLINK_TYPE ||
                           inode->base.inode_type == SQUASHFS_LSYMLINK_TYPE) {
                        // Read symlink
                        size_t size;
                        // read twice, once to find out right amount of memory to allocate
                        err = sqfs_readlink(&fs, inode, nullptr, &size);
                        if (err != SQFS_OK)
                            return false;

                        char symlinkTargetPath[size];
                        // then to populate the buffer
                        err = sqfs_readlink(&fs, inode, symlinkTargetPath, &size);
                        if (err != SQFS_OK)
                            return false;

                        // lookup symlink target path
                        *inode = rootInode;
                        err = sqfs_lookup_path(&fs, inode, symlinkTargetPath, &found);

                        if (!found || err != SQFS_OK)
                            return false;

                        // check if we fell into a symlinks cycle
                        auto ret = inodesVisited.insert(inode->base.inode_number);
                        if (!ret.second)
                            throw PayloadIteratorError("Symlinks loop found ");

                    }

                    return true;
                }


                /**
                * Read the current entry type from the underlying implementation.
                * @return Current entry type
                */
                appimage::core::PayloadEntryType readEntryType() const {
                    // squashfs traversal follows a DFS pattern but directories are "visited" twice, when they are reached and when they
                    // are left. This check is to properly identify the second scenario.
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

                /**
                * Read the current entry path from the underlying implementation.
                * @return Current entry path
                */
                std::string readEntryName() const {
                    if (trv.path != nullptr)
                        return trv.path;
                    else
                        return string();
                }

                /**
                 * Read the current entry link path from the underlying implementation.
                 * @return Current link entry path or an empty string if the entry is not a link.
                 * */
                std::string readEntryLink() {
                    // read the target link path size
                    size_t size;
                    auto err = sqfs_readlink(&fs, &currentInode, nullptr, &size);
                    if (err != SQFS_OK)
                        throw IOError("sqfs_readlink error");

                    char buf[size];

                    // read the target link in buf
                    err = sqfs_readlink(&fs, &currentInode, buf, &size);
                    if (err != SQFS_OK)
                        throw IOError("sqfs_readlink error");

                    // If the returned string is not NULL terminated a buffer overflow may occur, creating the string this way
                    // prevents it
                    return std::string(buf, buf + size - 1);
                }

                /**
                * Creates a directory at target.
                * @param target
                */
                void extractDir(const std::string& target) {
                    // The directory doesn't exists
                    if (access(target.c_str(), F_OK) == -1) {
                        // Create new directory with 755 permissions
                        if (mkdir(target.c_str(), 0755) == -1)
                            throw FileSystemError("mkdir error at " + target);
                    }
                }


                /**
                * extract the file pointed by <inode> contents at <target>
                * @param inode file
                * @param target path
                */
                void extractFile(sqfs_inode inode, const std::string& target) {
                    // open read stream
                    auto& istream = read();

                    // open write stream
                    ofstream targetFile(target);

                    // transfer data
                    targetFile << istream.rdbuf();
                    targetFile.close();

                    // set file stats
                    chmod(target.c_str(), inode.base.mode);
                }


                /**
                * extract the symlink pointed by <inode> at <target>
                * @param inode symlink
                * @param target path
                */
                void extractSymlink(sqfs_inode inode, const std::string& target) {
                    // read the target link in buf
                    int ret = unlink(currentEntryLink.c_str());
                    if (ret != 0 && errno != ENOENT)
                        throw IOError("unlink error at " + target);

                    ret = symlink(currentEntryLink.c_str(), target.c_str());
                    if (ret != 0)
                        throw IOError("symlink error at " + target);
                }
            };

            TraversalType2::TraversalType2(std::string path) : d(new Priv(path)) {
                // The traversal starts pointing to an empty entry, fetch first entry to be in a valid stated
                next();
            }

            TraversalType2::~TraversalType2() = default;

            void TraversalType2::next() {
                d->next();
            }


            bool TraversalType2::isCompleted() const {
                return d->isCompleted();
            }

            std::string TraversalType2::getEntryPath() const {
                return d->getCurrentEntryPath();
            }

            appimage::core::PayloadEntryType TraversalType2::getEntryType() const {
                return d->getCurrentEntryType();
            }

            void TraversalType2::extract(const std::string& target) {
                d->extract(target);
            }


            istream& TraversalType2::read() {
                return d->read();

            }

            string TraversalType2::getEntryLinkTarget() const {
                return d->getCurrentEntryLink();
            }
        }
    }
}
