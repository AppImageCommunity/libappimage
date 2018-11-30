#pragma once

#include <string>
#include <vector>
#include "AppImageContentFilesIterator.h"

namespace AppImage {
    /**
     * The image format determines how an AppImage is represented on disk. See the link below for
     * https://github.com/AppImage/AppImageSpec/blob/master/draft.md#image-format
     */
    enum Format {
        Unknown = -1,   // Not an AppImage file
        Legacy = 0,     // portable binaries that look and behave like AppImages but do not follow the standard
        Type1 = 1,      // https://github.com/AppImage/AppImageSpec/blob/master/draft.md#type-1-image-format
        Type2 = 2       // https://github.com/AppImage/AppImageSpec/blob/master/draft.md#type-2-image-format
    };

    class File {
    public:
        const std::string& getPath() const;

        Format getFormat() const;

        static Format getFormat(const std::string &path);

        virtual AppImageContentFilesIterator getContentFilesIterator() = 0;
    };
}
