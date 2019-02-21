/*
 * NOTE ON SQUASHFUSE:
 * It wasn't designed originally as a library and its headers are somehow broken.
 * Therefore they must be kept confined.
 *
 * keep squashfuse includes on top to avoid _POSIX_C_SOURCE redefinition warning
*/
extern "C" {
#include <squashfuse.h>
#include <squashfs_fs.h>

#include <sys/stat.h>
}

// local
#include "appimage/core/exceptions.h"
#include "StreambufType2.h"

using namespace appimage::core::impl;

StreambufType2::StreambufType2(sqfs* fs, sqfs_inode* inode, unsigned long size)
    : fs(fs), inode(inode), buffer(size) {
}

StreambufType2::StreambufType2(StreambufType2&& other) noexcept
    : fs(other.fs), inode(other.inode), buffer(std::move(other.buffer)) {

    // Reset the three read area pointers
    setg(other._M_in_beg, other._M_in_cur, other._M_in_end);
}

StreambufType2& StreambufType2::operator=(StreambufType2&& other) noexcept {
    fs = other.fs;
    inode = other.inode;
    buffer = std::move(other.buffer);

    // Reset the three read area pointers
    setg(other._M_in_beg, other._M_in_cur, other._M_in_end);
    return *this;
}

int StreambufType2::underflow() {
    // notify eof if the whole file was read
    if (bytes_already_read >= inode->xtra.reg.file_size) return traits_type::eof();

    // read next data chunk
    sqfs_off_t bytes_at_a_time = buffer.size();
    if (sqfs_read_range(fs, inode, (sqfs_off_t) bytes_already_read, &bytes_at_a_time, buffer.data()))
        throw IOError("sqfs_read_range error");

    bytes_already_read += bytes_at_a_time;

    // Update streambuf read pointers see <setg> doc
    setg(buffer.data(), buffer.data(), buffer.data() + bytes_at_a_time);

    // return the first char
    return traits_type::to_int_type(*gptr());
}
