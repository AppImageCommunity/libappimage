#pragma once

#include <memory>
#include <string>


namespace appimage {
    namespace desktop_integration {
        /**
         * @brief Integrator instances allow the integration and disintegration of AppImage with XDG compliant desktop
         * environments.
         *
         */
        class Integrator {
        public:
            explicit Integrator(const std::string& path);

            Integrator(const std::string& path, const std::string& xdgDataDir);

            virtual ~Integrator();

            void integrate();

            std::string getDesktopFilePath();

        private:
            class priv;

            std::unique_ptr<priv> d_ptr;   // opaque pointer
        };
    }
}
