#pragma once

#include <memory>
#include <streambuf>
#include <vector>

extern "C" {
#include <squashfuse.h>
#include <squashfs_fs.h>
}


namespace appimage {
    namespace core {
        namespace impl {
            /**
             * Provides a streambuf implementation for reading type 2 AppImages
             * by means of squashfuse.
             *
             * For more details about streambuf see https://gcc.gnu.org/onlinedocs/libstdc++/manual/streambufs.html
             */
            class streambuf_type_2 : public std::streambuf {
                struct sqfs fs;
                sqfs_inode inode;
                std::vector<char> buffer;
                unsigned long size;
                sqfs_off_t bytes_already_read = 0;

            public:
                /**
                 * Create an streambuf_type_2 object for reading the file pointed by <inode> at <fs>
                 * of size <size>
                 * @param fs
                 * @param inode
                 * @param size
                 */
                streambuf_type_2(sqfs fs, const sqfs_inode& inode, unsigned long size);

            protected:
                /**
                 * @brief  Fetches more data from the controlled sequence.
                 * See parenth method documentation.
                 * @return e first character from the <em>pending sequence</em>.
                 */
                int underflow() override;
            };
        }
    }
}

