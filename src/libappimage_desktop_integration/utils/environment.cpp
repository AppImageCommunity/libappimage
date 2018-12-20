// system
#include <cstdlib>

// local
#include "environment.h"


namespace appimage {
    namespace utils {

        std::string environment::operator[](std::string var) {
            const char* v = getenv(var.c_str());
            if (v != nullptr)
                return v;
            else return std::string();
        }

        bool environment::isSet(std::string var) {
            const char* v = getenv(var.c_str());
            return v != nullptr;
        }
    }
}

