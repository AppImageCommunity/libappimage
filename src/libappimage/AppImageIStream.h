#pragma once

#include <istream>
#include <memory>

namespace appimage {
    class AppImageIStream : public std::istream {
        std::shared_ptr<std::streambuf> streambuf;
    public:
        AppImageIStream(const std::shared_ptr<std::streambuf>& streambuf);
    };
}

