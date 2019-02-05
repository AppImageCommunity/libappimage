// local
#include <appimage/core/exceptions.h>
#include "StreambufType1.h"

using namespace appimage::core::impl;

int StreambufType1::underflow() {
    // Read line from original source
    auto bytesRead = archive_read_data(a, buffer.data(), size);

    // In case of error a value lower than 0 is returned
    if (bytesRead < 0)
        throw IOError(archive_error_string(a));

    // notify eof if nothing
    if (bytesRead == 0) return traits_type::eof();

    // Update streambuf read pointers see <setg> doc
    setg(buffer.data(), buffer.data(), buffer.data() + bytesRead);

    // return the first char
    return traits_type::to_int_type(*gptr());
}

StreambufType1::StreambufType1(archive* a, unsigned long size) : a(a), size(size), buffer(size) {}

StreambufType1::StreambufType1(StreambufType1&& other) noexcept
    : a(other.a), size(other.size), buffer(std::move(other.buffer)) {
    // Reset the three read area pointers
    setg(other._M_in_beg, other._M_in_cur, other._M_in_end);
}

StreambufType1& StreambufType1::operator=(StreambufType1&& other) noexcept {
    a = other.a;
    size = other.size;
    buffer = std::move(other.buffer);

    // Reset the three read area pointers
    setg(other._M_in_beg, other._M_in_cur, other._M_in_end);

    return *this;
}
