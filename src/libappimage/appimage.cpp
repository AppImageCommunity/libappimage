#include <iostream>

#include "appimage.h"
#include "MagicBytesChecker.h"
#include "AppImageType1Traversal.h"

appimage::appimage::appimage(const std::string& path) : path(path), format(getFormat(path)) {}

appimage::Format appimage::appimage::getFormat(const std::string& path) {
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

const std::string& appimage::appimage::getPath() const {
    return path;
}

appimage::Format appimage::appimage::getFormat() const {
    return format;
}

appimage::appimage::~appimage() {}

appimage::AppImageIterator appimage::appimage::files() {
    return AppImageIterator(path, format);
}
