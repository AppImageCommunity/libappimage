#pragma once
// system
#include <istream>
#include <memory>

namespace appimage {
    namespace core {
        /**
         * @brief Convenience wrapper around std::streambuf to allow the creation of std::istream instances from the files
         * contained inside a given AppImage.
         *
         * This class should be instanced by the AppImage traversal derivatives by providing the right std::streambuf
         * derivative for a given AppImage format.
         *
         * @related traversal.h
         */
        class file_istream : public std::istream {
            std::shared_ptr<std::streambuf> streambuf;
        public:
            file_istream(const std::shared_ptr<std::streambuf>& streambuf);
        };
    }
}

