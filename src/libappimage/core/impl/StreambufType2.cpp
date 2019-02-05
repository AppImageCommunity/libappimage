extern "C" {
// system
#include <sys/stat.h>

// libraries
#include <nonstd.h>
#include <squashfuse.h>
#include <squashfs_fs.h>
}


#include "appimage/core/exceptions.h"
#include "StreambufType2.h"

using namespace appimage::core::impl;

StreambufType2::StreambufType2(sqfs fs, const sqfs_inode& inode, unsigned long size)
    : fs(fs), inode(inode), size(size) {
    buffer.resize(size); // allocate buffer memory
}

StreambufType2::StreambufType2(StreambufType2&& other) noexcept
    : fs(other.fs), inode(other.inode), size(other.size), buffer(std::move(other.buffer)) {

    // Reset the three read area pointers
    setg(other._M_in_beg, other._M_in_cur, other._M_in_end);
}

StreambufType2& StreambufType2::operator=(StreambufType2&& other) noexcept {
    fs = other.fs;
    inode = other.inode;
    size = other.size;
    buffer = std::move(other.buffer);

    // Reset the three read area pointers
    setg(other._M_in_beg, other._M_in_cur, other._M_in_end);
    return *this;
}

int StreambufType2::underflow() {
    // notify eof if the whole file was read
    if (bytes_already_read >= inode.xtra.reg.file_size) return traits_type::eof();

    // read next data chunk
    sqfs_off_t bytes_at_a_time = size;
    if (sqfs_read_range(&fs, &inode, (sqfs_off_t) bytes_already_read, &bytes_at_a_time, buffer.data()))
        throw IOError("sqfs_read_range error");

    bytes_already_read += bytes_at_a_time;

    // Update streambuf read pointers see <setg> doc
    setg(buffer.data(), buffer.data(), buffer.data() + bytes_at_a_time);

    // return the first char
    return traits_type::to_int_type(*gptr());
}
