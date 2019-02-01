#pragma once

// system
#include <stdexcept>

namespace appimage {
    namespace core {
        /**
         * Generic Error that can be thrown by AppImage procedures.
         */
        class AppImageError : public std::runtime_error {
        public:
            explicit AppImageError(const std::string& what) : runtime_error(what) {}
        };

        /**
         * Throw in case of missing files, insufficient permissions and other file system related
         * errors.
         */
        class FileSystemError : public AppImageError {
        public:
            explicit FileSystemError(const std::string& what) : AppImageError(what) {}
        };

        /**
         * Throw in case of failure in a read or write operation.
         */
        class IOError : public AppImageError {
        public:
            explicit IOError(const std::string& what) : AppImageError(what) {}
        };
    }
};
