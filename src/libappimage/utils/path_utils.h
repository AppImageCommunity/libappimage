#pragma once

// system
#include <string>
#include <filesystem>

namespace appimage {
    namespace utils {
        /**
         * Prepends 'file://' to a local path string if required.
         * @param path
         * @return
         */
        std::string pathToURI(const std::string& path);

        /**
         * @brief Provides a MD5 hash that identifies a file given its <path>.
         *
         * Implementation of the thumbnail filename hash function available at:
         * https://specifications.freedesktop.org/thumbnail-spec/thumbnail-spec-latest.html#THUMBSAVE
         *
         * It's may be used to identify files that are related to a given AppImage at a given location.
         *
         * @param path
         * @return file hash
         */
        std::string hashPath(const std::filesystem::path& path);
    }
}
