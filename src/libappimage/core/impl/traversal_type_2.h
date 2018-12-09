#pragma once
#include <memory>

extern "C" {
#include <squashfuse.h>
#include <squashfs_fs.h>
}

#include "core/traversal.h"

namespace appimage {
    namespace core {
        namespace impl {
            class traversal_type_2 : public traversal {
                std::string path;
                bool completed = false;
                struct sqfs fs;
                sqfs_traverse trv;
                sqfs_inode_id rootInodeId;

                std::shared_ptr<std::istream> appImageIStream;
            public:
                traversal_type_2(std::string path);

                ~traversal_type_2() override;

                void next() override;

                bool isCompleted() override;

                std::string getEntryName() override;

                void extract(const std::string& target) override;

                std::istream& read() override;

            private:
                void extractDir(const std::string& target);

                void extractFile(sqfs_inode inode, const std::string& target);

                void extractSymlink(sqfs_inode inode, const std::string& target);

                static sqfs_err sqfs_stat(sqfs* fs, sqfs_inode* inode, struct stat* st);

                /**
                 * If the <inode> points to a symlink it is followed until a regular file is found.
                 * This method is aware of symlink loops and will fail properly in such case.
                 * @param inode [RETURN PARAMETER] will be filled with a regular file inode. It cannot be NULL
                 * @return succeed true if the file is found, otherwise false
                 */
                bool resolve_symlink(sqfs_inode* inode);
            };
        }
    }
}
