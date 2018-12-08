#pragma once

#include <vector>
#include <streambuf>

#include <archive.h>

namespace appimage {
    namespace core {
        namespace impl {
            class streambuf_type1 : public std::streambuf {
                unsigned long size;
                std::vector<char> buffer;
                struct archive* a = {nullptr};
            protected:
            public:
                streambuf_type1(archive* a, unsigned long size);

            protected:
                int underflow() override;
            };
        }
    }
}
