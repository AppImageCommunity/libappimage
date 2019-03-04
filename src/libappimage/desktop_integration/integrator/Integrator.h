#pragma once

// system
#include <memory>
#include <string>

// local
#include <appimage/core/AppImage.h>

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            /**
             * @brief Integrator instances allow the integration and disintegration of AppImage with XDG compliant desktop
             * environments.
             */
            class Integrator {
            public:

                /**
                 * Create an Integrator instance with a custom XDG_DATA_HOME.
                 * @param appImage
                 * @param xdgDataHome
                 */
                explicit Integrator(const core::AppImage& appImage, const std::string& xdgDataHome,
                                    const std::string& vendorPrefix);

                // Creating copies of this object is not allowed
                Integrator(Integrator& other) = delete;

                // Creating copies of this object is not allowed
                Integrator& operator=(Integrator& other) = delete;

                virtual ~Integrator();

                /**
                 * @brief Perform the AppImage integration into the Desktop Environment
                 *
                 * Extract the main application desktop entry, icons and mime type packages. Modifies their content to
                 * properly match the AppImage file location and deploy them into the use XDG_DATA_HOME appending a
                 * prefix to each file. Such prefix is composed as "<vendor id>_<appimage_path_md5>_<old_file_name>"
                 */
                void integrate();

            private:
                class Priv;
                std::unique_ptr<Priv> priv;   // opaque pointer
            };
        }

    }
}
