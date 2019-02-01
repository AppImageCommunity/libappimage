#pragma once

// system
#include <vector>
#include <streambuf>

// libraries
#include <archive.h>

namespace appimage {
    namespace core {
        namespace impl {
            /**
             * Provides a streambuf implementation for reading type 1 AppImages
             * by means of libarchive.
             *
             * For more details about streambuf see https://gcc.gnu.org/onlinedocs/libstdc++/manual/streambufs.html
             */
            class StreambufType1 : public std::streambuf {
                unsigned long size;
                std::vector<char> buffer;
                struct archive* a = {nullptr};

            public:
                /**
                 * Create an streambuf_type_1 object from an archive <a> pointer
                 * with a buffer size of <size>
                 * @param a opened archive struct from libarchive
                 * @param size buffer size
                 */
                StreambufType1(archive* a, unsigned long size);

                // Creating copies of this object is not allowed
                StreambufType1(StreambufType1& other) = delete;

                // Creating copies of this object is not allowed
                StreambufType1& operator=(StreambufType1& other) = delete;

            protected:
                /**
                 * @brief  Fetches more data from the controlled sequence.
                 * See parenth method documentation.
                 * @return e first character from the <em>pending sequence</em>.
                 */
                int underflow() override;
            };
        }
    }
}
