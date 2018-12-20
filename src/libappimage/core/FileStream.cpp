#include "FileStream.h"

appimage::core::FileStream::FileStream(const std::shared_ptr<std::streambuf>& streambuf)
    : basic_istream(streambuf.get()), streambuf(streambuf) {}
