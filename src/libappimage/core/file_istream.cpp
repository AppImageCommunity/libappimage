#include "file_istream.h"

appimage::core::file_istream::file_istream(const std::shared_ptr<std::streambuf>& streambuf)
    : basic_istream(streambuf.get()), streambuf(streambuf) {}
