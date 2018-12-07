#include <istream>
#include <vector>
#include "AppImageDummyTraversal.h"
#include "AppImageDummyStreamBuffer.h"

void AppImage::AppImageDummyTraversal::next() {

}

bool AppImage::AppImageDummyTraversal::isCompleted() {
    return true;
}

std::string AppImage::AppImageDummyTraversal::getEntryName() {
    return std::string();
}

void AppImage::AppImageDummyTraversal::extract(const std::string& target) {}

std::istream& AppImage::AppImageDummyTraversal::read() {
    auto dummyStreamBuffer = new AppImageDummyStreamBuffer();
    fileStream.reset(new std::istream(dummyStreamBuffer));

    return *fileStream;
}
