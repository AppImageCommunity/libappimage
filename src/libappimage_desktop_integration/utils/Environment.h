#pragma once

// system
#include <string>

namespace appimage {
    namespace utils {
        /**
         * @brief utility class to provide a map alike access to the system environment variables
         */
        class Environment {
        public:
            /**
             * Read the environment variable named <var>
             * @param var
             * @return the environment variable value or an empty string
             */
            std::string operator[](std::string var);

            /**
             * Checks if environment variable named <var> exists
             * @param var
             * @return true if exists, false otherwise
             */
            bool isSet(std::string var);
        };
    }
}
