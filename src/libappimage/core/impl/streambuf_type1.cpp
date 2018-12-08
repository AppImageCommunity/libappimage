#include "streambuf_type1.h"

using namespace appimage::core::impl;

int streambuf_type1::underflow() {
    // Read line from original buffer
    auto bytesRead = archive_read_data(a, buffer.data(), size);
    if (!bytesRead) return traits_type::eof();

    setg(buffer.data(), buffer.data(), buffer.data() + bytesRead);
    return traits_type::to_int_type(*gptr());
}

streambuf_type1::streambuf_type1(archive* a, unsigned long size) : a(a), size(size) {
    buffer.resize(size);
}
