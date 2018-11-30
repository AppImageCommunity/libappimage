#pragma once

#include <ostream>
#include "AppImageContentFile.h"
namespace AppImage {
    class AppImageContentFilesIterator {
    public:
        virtual const AppImageContentFile& operator*() = 0;

        virtual const AppImageContentFilesIterator& operator++() = 0;

        virtual const AppImageContentFilesIterator& operator!=(const AppImageContentFilesIterator& other) = 0;
    };

}
