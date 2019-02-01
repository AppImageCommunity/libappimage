#pragma once

// system
#include <string>
#include <stdexcept>
#include <dlfcn.h>

namespace appimage {
    namespace utils {
        class DLHandleError : public std::runtime_error {
        public:
            explicit DLHandleError(const std::string& what) : runtime_error(what) {}
        };

        /**
         * @brief Dynamic Loader wrapper
         *
         * Allow to dynamically load a library.
         */
        class DLHandle {
        public:
            explicit DLHandle(const std::string& libName) : libName(libName) {
                handle = dlopen(libName.c_str(), RTLD_LAZY | RTLD_NODELETE);

                if (!handle)
                    throw DLHandleError("Unable to load " + libName);
            }

            virtual ~DLHandle() = default;

            template<typename T>
            void loadSymbol(T& symbol, const std::string& symbolName) {
                symbol = (T) dlsym(handle, symbolName.c_str());

                if (symbol == nullptr)
                    throw DLHandleError("Unable to load " + libName + " symbol: " + symbolName);
            }

        private:
            std::string libName;
            void* handle{};
        };
    }
}

