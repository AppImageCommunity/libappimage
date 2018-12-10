#pragma once

#include <streambuf>
#include <iostream>

namespace appimage {
    namespace core {
        namespace impl {
            /**
             * Provide a fallback streambuf implementation to be used in cases where a given file entry has not
             * content because it's a directory or because an error.
             */
            class streambuf_fallback : public std::streambuf {};
        }
    }
}
