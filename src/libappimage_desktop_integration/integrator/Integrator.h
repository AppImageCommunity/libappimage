#pragma once

#include <memory>
#include <string>


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
                 * Create an integrator instance for the AppImage pointed by <path>
                 * @param path
                 */
                explicit Integrator(const std::string& path);

                /**
                 * Create an Integrator instance with a custom XDG_DATA_DIR.
                 * @param path
                 * @param xdgDataDir
                 */
                Integrator(const std::string& path, const std::string& xdgDataDir);

                virtual ~Integrator();

                /**
                 * @brief Perform the AppImage integration into the Desktop Environment
                 *
                 * Extract the main application desktop entry, icons and mime type packages. Modifies their content to
                 * properly match the AppImage file location and deploy them into the use XDG_DATA_DIR appending a
                 * prefix to each file. Such prefix is composed as "<vendor id>_<appimage_path_md5>_<old_file_name>"
                 */
                void integrate();

            private:
                struct Priv;
                std::unique_ptr<Priv> priv;   // opaque pointer
            };
        }

    }
}
