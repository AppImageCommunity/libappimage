#pragma once

#include <streambuf>
#include <iostream>

namespace appimage {
    class AppImageDummyStreamBuffer : public std::streambuf {};
}
