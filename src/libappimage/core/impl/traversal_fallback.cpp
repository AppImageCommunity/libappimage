#include <istream>
#include <vector>

#include "streambuf_fallback.h"
#include "traversal_fallback.h"

using namespace appimage::core::impl;


traversal_fallback::traversal_fallback() = default;

void traversal_fallback::next() {}

bool traversal_fallback::isCompleted() {
    return true;
}

std::string traversal_fallback::getEntryName() {
    return std::string();
}

void traversal_fallback::extract(const std::string& target) {}

std::istream& traversal_fallback::read() {
    auto defaultStreambuf = new streambuf_fallback();
    fileStream.reset(new std::istream(defaultStreambuf));

    return *fileStream;
}
