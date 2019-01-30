#pragma once
// system
#include <memory>

extern "C" {
// libraries
#include <squashfuse.h>
#include <squashfs_fs.h>
}

// local
#include "core/Traversal.h"
#include "PayloadIStream.h"
#include "StreambufType2.h"

namespace appimage {
    namespace core {
        namespace impl {
            /**
             * Provides an implementation of the traversal class for type 2 AppImages. It's based on squashfuse.
             * The current implementation has the following limitations: READONLY, ONE WAY, SINGLE PASS.
             *
             * See the base class for more details.
             */
            class TraversalType2 : public Traversal {
                std::string path;
                bool completed = false;
                struct sqfs fs;
                sqfs_traverse trv;
                sqfs_inode_id rootInodeId;

                // Current entry data cache
                entry::Type currentEntryType;
                std::string currentEntryPath;
                std::string currentEntryLink;

                PayloadIStream entryIStream;
                std::unique_ptr<StreambufType2> entryStreamBuf;
            public:
                explicit TraversalType2(std::string path);

                // Creating copies of this object is not allowed
                TraversalType2(TraversalType2& other) = delete;

                // Creating copies of this object is not allowed
                TraversalType2& operator=(TraversalType2& other) = delete;

                ~TraversalType2() override;

                void next() override;

                bool isCompleted() const override;

                std::string getEntryName() const override;

                std::string getEntryLink() const override;

                entry::Type getEntryType() const override;

                void extract(const std::string& target) override;

                std::istream& read() override;

            private:
                /**
                 * Creates a directory at target.
                 * @param target
                 */
                void extractDir(const std::string& target);

                /**
                 * extract the file pointed by <inode> contents at <target>
                 * @param inode file
                 * @param target path
                 */
                void extractFile(sqfs_inode inode, const std::string& target);

                /**
                 * extract the symlink pointed by <inode> at <target>
                 * @param inode symlink
                 * @param target path
                 */
                void extractSymlink(sqfs_inode inode, const std::string& target);

                /**
                 * Fill in a stat structure with the data from <inode>. Does not set st_ino.
                 * @param fs
                 * @param inode
                 * @param st output paramether
                 * @return SQFS_OK if al goes well
                 */
                static sqfs_err sqfs_stat(sqfs* fs, sqfs_inode* inode, struct stat* st);

                /**
                 * If the <inode> points to a symlink it is followed until a regular file is found.
                 * This method is aware of symlink loops and will fail properly in such case.
                 * @param inode [RETURN PARAMETER] will be filled with a regular file inode. It cannot be NULL
                 * @return succeed true if the file is found, otherwise false
                 */
                bool resolve_symlink(sqfs_inode* inode);

                /**
                 * Read the current entry type from the underlying implementation.
                 * @return Current entry type
                 */
                entry::Type readEntryType() const;

                /**
                 * Read the current entry path from the underlying implementation.
                 * @return Current entry path
                 */
                std::string readEntryName() const;

                /**
                 * Read the current entry link path from the underlying implementation.
                 * @return Current link entry path or an empty string if the entry is not a link.
                 */
                std::string readEntryLink();
            };
        }
    }
}
