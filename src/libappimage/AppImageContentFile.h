#pragma once

#include <string>
#include <vector>

namespace AppImage {
    enum ContentFileType {

    };

    class AppImageContentFile {
    public:
        virtual ContentFileType getType() const = 0;

        virtual const std::string& getPath() const = 0;

        /**
         * Extract the file to the given path
         * @param path
         */
        virtual void extractTo(const std::string& path) = 0;

        /**
         * Read the whole file into memory
         * @return file contents
         */
        virtual std::vector<char> read() = 0;
    };
}
