// system
#include <istream>
#include <vector>

// local
#include "streambuf_fallback.h"
#include "traversal_fallback.h"

using namespace appimage::core::impl;

void traversal_fallback::next() {}

bool traversal_fallback::isCompleted() {
    return true;
}

std::string traversal_fallback::getEntryName() {
    return std::string();
}

void traversal_fallback::extract(const std::string& target) {}

std::istream& traversal_fallback::read() {
    // provide a dummy streambuf to avoid crashes
    if (!fileStream) {
        auto defaultStreambuf = new streambuf_fallback();
        fileStream.reset(new std::istream(defaultStreambuf));
    }

    return *fileStream;
}
