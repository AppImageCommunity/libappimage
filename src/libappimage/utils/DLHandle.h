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
            /**
             * Load <libName> with the given <mode> flags. For details about he allowed flags
             * see the dlopen doc.
             * @param libName
             * @param mode
             */
            explicit DLHandle(const std::string& libName, int mode) : libName(libName) {
                handle = dlopen(libName.c_str(), mode);

                if (!handle)
                    throw DLHandleError("Unable to load " + libName);
            }

            virtual ~DLHandle() {
                dlclose(handle);
            };

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

