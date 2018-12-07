#pragma once

#include <streambuf>
#include <iostream>

namespace AppImage {
    class AppImageDummyStreamBuffer : public std::streambuf {};
}
