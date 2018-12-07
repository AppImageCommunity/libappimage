#pragma once

#include <istream>
#include <boost/shared_ptr.hpp>

namespace AppImage {
    class AppImageFileStream : public std::istream {
        std::shared_ptr<std::streambuf> streambuf;
    public:
        AppImageFileStream(const std::shared_ptr<std::streambuf>& streambuf);
    };
}

