// system
#include <istream>
#include <vector>

// local
#include "StreambufFallback.h"
#include "TraversalFallback.h"

using namespace appimage::core::impl;

void TraversalFallback::next() {}

bool TraversalFallback::isCompleted() const {
    return true;
}

std::string TraversalFallback::getEntryName() const {
    return std::string();
}

void TraversalFallback::extract(const std::string& target) {}

std::istream& TraversalFallback::read() {
    // provide a dummy streambuf to avoid crashes
    if (!fileStream) {
        auto defaultStreambuf = new StreambufFallback();
        fileStream.reset(new std::istream(defaultStreambuf));
    }

    return *fileStream;
}

appimage::core::entry::Type TraversalFallback::getEntryType() const {
    return entry::UNKNOWN;
}

std::string TraversalFallback::getEntryLink() const {
    return std::string();
}
