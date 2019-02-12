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
            public:
                explicit TraversalType2(std::string path);

                // Creating copies of this object is not allowed
                TraversalType2(TraversalType2& other) = delete;

                // Creating copies of this object is not allowed
                TraversalType2& operator=(TraversalType2& other) = delete;

                ~TraversalType2();

                void next() override;

                bool isCompleted() const override;

                std::string getEntryPath() const override;

                std::string getEntryLinkTarget() const override;

                PayloadEntryType getEntryType() const override;

                void extract(const std::string& target) override;

                std::istream& read() override;

            private:
                std::string path;
                bool completed = false;
                struct sqfs fs;
                sqfs_traverse trv;
                sqfs_inode_id rootInodeId;
                sqfs_inode currentInode;

                // Current entry data cache
                PayloadEntryType currentEntryType;
                std::string currentEntryPath;
                std::string currentEntryLink;

                PayloadIStream entryIStream;
                std::unique_ptr<StreambufType2> entryStreamBuf;

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
                 * If the <inode> points to a symlink it is followed until a regular file is found.
                 * This method is aware of symlink loops and will fail properly in such case.
                 * @param inode [RETURN PARAMETER] will be filled with a regular file inode. It cannot be NULL
                 * @return succeed true if the file is found, otherwise false
                 */
                bool resolveSymlink(sqfs_inode* inode);

                /**
                 * Read the current entry type from the underlying implementation.
                 * @return Current entry type
                 */
                PayloadEntryType readEntryType() const;

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

                /**
                 * @return current sqfs_inode
                 */
                sqfs_inode readInode();
            };
        }
    }
}
