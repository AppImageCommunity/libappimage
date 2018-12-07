#include "AppImageFileStream.h"

AppImage::AppImageFileStream::AppImageFileStream(const std::shared_ptr<std::streambuf>& streambuf)
    : basic_istream(streambuf.get()), streambuf(streambuf) {}
