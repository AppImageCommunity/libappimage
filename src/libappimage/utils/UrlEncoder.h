#pragma once

// system
#include <string>
#include <sstream>


namespace appimage {
    namespace utils {
        /**
         * Provides a minimal implementation of the Uniform Resource Identifiers (RFC 2396)
         * See: https://tools.ietf.org/html/rfc2396
         */
        class UrlEncoder {
        public:
            /**
             * Scape chars in <value> according to RFC 2396
             * @param value
             * @return
             */
            static std::string encode(const std::string& value);
        };
    }
}
