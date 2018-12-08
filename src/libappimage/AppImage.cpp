#include <iostream>

#include "AppImage.h"
#include "MagicBytesChecker.h"
#include "AppImageType1Traversal.h"

appimage::AppImage::AppImage(const std::string& path) : path(path), format(getFormat(path)) {}

appimage::Format appimage::AppImage::getFormat(const std::string& path) {
    MagicBytesChecker magicBytesChecker(path);
    if (magicBytesChecker.hasAppImageType1Signature())
        return Type1;
    if (magicBytesChecker.hasAppImageType2Signature())
        return Type2;

    if (magicBytesChecker.hasIso9660Signature() && magicBytesChecker.hasElfSignature()) {
        std::cerr << "WARNING: " << path << " seems to be a Type 1 AppImage without magic bytes." << std::endl;
        return Type1;
    }

    return Unknown;
}

const std::string& appimage::AppImage::getPath() const {
    return path;
}

appimage::Format appimage::AppImage::getFormat() const {
    return format;
}

appimage::AppImage::~AppImage() {}

appimage::AppImageIterator appimage::AppImage::files() {
    return AppImageIterator(path, format);
}
