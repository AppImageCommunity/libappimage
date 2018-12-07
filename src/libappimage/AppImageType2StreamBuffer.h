#pragma once

#include <memory>
#include <streambuf>
#include <vector>

extern "C" {
#include <squashfuse.h>
#include <squashfs_fs.h>
}


namespace AppImage {
    class AppImageType2StreamBuffer : public std::streambuf {
        struct sqfs fs;
        sqfs_inode inode;
        std::vector<char> buffer;
        unsigned long size;
        sqfs_off_t bytes_already_read = 0;

    public:
        AppImageType2StreamBuffer(sqfs fs, const sqfs_inode& inode, unsigned long size);

    protected:
        int underflow() override;
    };
}

