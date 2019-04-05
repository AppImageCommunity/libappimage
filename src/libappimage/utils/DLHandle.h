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
            explicit DLHandle(const std::string& libName, int mode) : handle(nullptr), libName(libName) {
                handle = dlopen(libName.c_str(), mode);

                if (!handle)
                    throw DLHandleError("Unable to load " + libName);
            }

            /**
             * Load one of the libraries listed in <libNames> with the given <mode> flags. For details about
             * the allowed flags see the dlopen doc.
             * @param libName
             * @param mode
             */
            explicit DLHandle(std::initializer_list<const std::string> libNames, int mode) : handle(nullptr) {
                for (const auto& item: libNames) {
                    handle = dlopen(item.c_str(), mode);
                    if (handle) {
                        libName = item;
                        break;
                    }
                }

                if (!handle) {
                    std::string libNamesStr;
                    for (const auto& item: libNames)
                        libNamesStr += " " + item;

                    throw DLHandleError("Unable to load any of: " + libNamesStr);
                }
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

