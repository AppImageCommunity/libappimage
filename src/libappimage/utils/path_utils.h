#pragma once

// system
#include <string>

// libraries
#include <boost/filesystem.hpp>

namespace appimage {
    namespace utils {
        /**
         * Appends 'file://' to a local path string if required.
         * @param path
         * @return
         */
        std::string pathToUri(const std::string& path);

        /**
         * @brief Provides a MD5 hash that identifies a file given its <path>.
         *
         * Implementation of the thumbnail filename hashfunction available at:
         * https://specifications.freedesktop.org/thumbnail-spec/thumbnail-spec-latest.html#THUMBSAVE
         *
         * It's may be used to identify files that are related to a given AppImage at a given location.
         *
         * @param path
         * @return file hash
         */
        std::string hashPath(const boost::filesystem::path& path);
    }
}
