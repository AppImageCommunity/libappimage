#pragma once

// system
#include <vector>
#include <memory>

namespace appimage {
    namespace utils {

        /**
         * Provide the image manipulation functions required by libappimage, nothing more.
         * Currently are supported two image formats: png and svg. Those formats are the
         * ones recommended for creating icons at the FreeDesktop Icon Theme Specification.
         * See: https://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
         *
         * This implementation uses libcairo and librsvg as backend. Those libraries are
         * dynamically loaded at runtime so they are not required for building (or linking) the
         * binaries.
         */
        class IconHandle {
        public:
            /**
             * Create a IconHandle instance from <data>
             * @param data
             * @throw IconHandleError in case of a backend error or an unsupported image format
             */
            explicit IconHandle(std::vector<char>& data);

            /**
             * Create an IconHandle from a the file pointed by <path>
             * @param path
             */
            explicit IconHandle(const std::string &path);

            /**
             * @brief Save the icon to <path> with <format>.
             *
             * @param path target path
             * @throw IconHandleError in case of error
             */
            void save(const std::string& path, const std::string& format = "png");

            /**
             * @return the icon size
             */
            int getSize();

            /**
             * @brief Set a new size to the Icon.
             * @param size
             */
            void setSize(int size);

            /**
             * @return the image format ("png" or "svg")
             */
            std::string format();

            virtual ~IconHandle();

        private:
            class Priv;

            std::unique_ptr<Priv> d;
        };

        class IconHandleError : public std::runtime_error {
        public:
            explicit IconHandleError(const std::string& what);
        };
    }

}
