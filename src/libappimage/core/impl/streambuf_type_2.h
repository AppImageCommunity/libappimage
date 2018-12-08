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
            class streambuf_type_2 : public std::streambuf {
                struct sqfs fs;
                sqfs_inode inode;
                std::vector<char> buffer;
                unsigned long size;
                sqfs_off_t bytes_already_read = 0;

            public:
                streambuf_type_2(sqfs fs, const sqfs_inode& inode, unsigned long size);

            protected:
                int underflow() override;
            };
        }
    }
}

