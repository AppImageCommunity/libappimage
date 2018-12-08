#include "AppImageIStream.h"

appimage::AppImageIStream::AppImageIStream(const std::shared_ptr<std::streambuf>& streambuf)
    : basic_istream(streambuf.get()), streambuf(streambuf) {}
