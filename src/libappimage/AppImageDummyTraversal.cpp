#include <istream>
#include <vector>
#include "AppImageDummyTraversal.h"
#include "AppImageDummyStreamBuffer.h"

void appimage::AppImageDummyTraversal::next() {

}

bool appimage::AppImageDummyTraversal::isCompleted() {
    return true;
}

std::string appimage::AppImageDummyTraversal::getEntryName() {
    return std::string();
}

void appimage::AppImageDummyTraversal::extract(const std::string& target) {}

std::istream& appimage::AppImageDummyTraversal::read() {
    auto dummyStreamBuffer = new AppImageDummyStreamBuffer();
    fileStream.reset(new std::istream(dummyStreamBuffer));

    return *fileStream;
}
