#pragma once

#include <streambuf>
#include <iostream>

namespace appimage {
    namespace core {
        namespace impl {
            class streambuf_fallback : public std::streambuf {};
        }
    }
}
