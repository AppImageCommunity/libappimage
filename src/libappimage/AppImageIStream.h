#pragma once

#include <istream>
#include <boost/shared_ptr.hpp>

namespace AppImage {
    class AppImageIStream : public std::istream {
        std::shared_ptr<std::streambuf> streambuf;
    public:
        AppImageIStream(const std::shared_ptr<std::streambuf>& streambuf);
    };
}

