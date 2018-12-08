#pragma once

#include <istream>
#include <memory>

namespace appimage {
    namespace core {
        class file_istream : public std::istream {
            std::shared_ptr<std::streambuf> streambuf;
        public:
            file_istream(const std::shared_ptr<std::streambuf>& streambuf);
        };
    }
}

