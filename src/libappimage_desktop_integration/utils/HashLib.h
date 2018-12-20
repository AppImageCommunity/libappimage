#pragma once

#include <istream>
namespace appimage {
    namespace utils {
        /**
         * C++ wrapper around the bare C hashing algorithms implementations
         */
        class HashLib {
        public:
            /**
             * Convenience function to compute md5 sums from a std::istream
             * @param data
             * @return md5 sum on success, empty string otherwise
             */
            static std::vector<uint8_t> md5(std::istream& data);

            /**
             * Generates an hexadecimal representation of the values at <digest>
             * @param digest
             * @return hexadecimal representation of the values at <digest>
             */
            static std::string toHex(std::vector<uint8_t> digest);
        };
    }
}
