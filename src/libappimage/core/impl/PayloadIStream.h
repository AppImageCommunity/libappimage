#pragma once
// system
#include <istream>
#include <memory>

namespace appimage {
    namespace core {
        namespace impl {


            /**
             * @brief Convenience wrapper around std::streambuf to allow the creation of std::istream instances from the files
             * contained inside a given AppImage.
             *
             * @related traversal.h
             */
            class PayloadIStream : public std::istream {
            public:
                friend class TraversalType1;
                friend class TraversalType2;

                PayloadIStream() = default;

                // Creating copies of this object is not allowed
                PayloadIStream(PayloadIStream& other) = delete;

                // Creating copies of this object is not allowed
                PayloadIStream& operator=(PayloadIStream& other) = delete;
            };
        }
    }
}
