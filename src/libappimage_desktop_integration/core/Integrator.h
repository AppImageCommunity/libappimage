#pragma once

#include <memory>
#include <string>


namespace appimage {
    namespace core {
        /**
         * @brief Integrator instances allow the integration and disintegration of AppImage with XDG compliant desktop
         * environments.
         *
         */
        class integrator {
        public:
            explicit integrator(const std::string& path);

            integrator(const std::string& path, const std::string& xdgDataDir);

            virtual ~integrator();

            void integrate();

            std::string getDesktopFilePath();

        private:
            class priv;

            std::unique_ptr<priv> d_ptr;   // opaque pointer
        };
    }
}
