#pragma once

// system
#include <istream>
#include <vector>


namespace appimage {
    namespace utils {
        /**
         * C++ wrapper around the bare C hashing algorithms implementations
         */
        namespace hashlib {
            /**
             * Convenience function to compute md5 sums from a std::istream
             * @param data
             * @return md5 sum on success, empty string otherwise
             */
            std::vector<uint8_t> md5(std::istream& data);

            /**
             * Convenience function to compute md5 sums from a std::string
             * @param data
             * @return md5 sum on success, empty string otherwise
             */
            std::vector<uint8_t> md5(const std::string& data);

            /**
             * Generates an hexadecimal representation of the values at <digest>
             * @param digest
             * @return hexadecimal representation of the values at <digest>
             */
            std::string toHex(std::vector<uint8_t> digest);
        };
    }
}
