extern "C" {
#include <sys/stat.h>

#include <nonstd.h>
#include <squashfuse.h>
#include <squashfs_fs.h>
}


#include "AppImageType2StreamBuffer.h"
#include "AppImageErrors.h"


AppImage::AppImageType2StreamBuffer::AppImageType2StreamBuffer(sqfs fs, const sqfs_inode& inode, unsigned long size)
    : fs(fs), inode(inode), size(size) {
    buffer.resize(size);
}

int AppImage::AppImageType2StreamBuffer::underflow() {
    if (bytes_already_read >= inode.xtra.reg.file_size) return traits_type::eof();

    sqfs_off_t bytes_at_a_time = size;
    if (sqfs_read_range(&fs, &inode, (sqfs_off_t) bytes_already_read, &bytes_at_a_time, buffer.data()))
        throw AppImageReadError("sqfs_read_range error");

    bytes_already_read += bytes_at_a_time;

    setg(buffer.data(), buffer.data(), buffer.data() + bytes_at_a_time);
    return traits_type::to_int_type(*gptr());
}
