#include "AppImageType1StreamBuffer.h"

#include <archive.h>

int AppImageType1StreamBuffer::underflow() {
    // Read line from original buffer
    auto bytesRead = archive_read_data(a, buffer.data(), size);
    if (!bytesRead) return traits_type::eof();

    setg(buffer.data(), buffer.data(), buffer.data() + bytesRead);
    return traits_type::to_int_type(*gptr());
}

AppImageType1StreamBuffer::AppImageType1StreamBuffer(archive* a, unsigned long size) : a(a), size(size) {
    buffer.resize(size);
}
