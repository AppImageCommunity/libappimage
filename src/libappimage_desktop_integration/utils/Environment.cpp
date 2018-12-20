// system
#include <cstdlib>

// local
#include "Environment.h"


namespace appimage {
    namespace utils {

        std::string Environment::operator[](std::string var) {
            const char* v = getenv(var.c_str());
            if (v != nullptr)
                return v;
            else return std::string();
        }

        bool Environment::isSet(std::string var) {
            const char* v = getenv(var.c_str());
            return v != nullptr;
        }
    }
}

