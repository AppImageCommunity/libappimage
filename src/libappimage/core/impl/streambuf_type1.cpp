#include "streambuf_type1.h"

using namespace appimage::core::impl;

int streambuf_type1::underflow() {
    // Read line from original source
    auto bytesRead = archive_read_data(a, buffer.data(), size);

    // notify eof if nothing was read (real eof or error)
    if (!bytesRead) return traits_type::eof();

    // Update streambuf read pointers see <setg> doc
    setg(buffer.data(), buffer.data(), buffer.data() + bytesRead);

    // return the first char
    return traits_type::to_int_type(*gptr());
}

streambuf_type1::streambuf_type1(archive* a, unsigned long size) : a(a), size(size) {
    buffer.resize(size); // allocate buffer size
}
