#pragma once

#include <memory>
#include <string>

namespace appimage {
    namespace desktop_integration {
        namespace core {
            class integrator {
            public:
                explicit integrator(const std::string& path);

                integrator(std::string path, std::string xdg_data_dir);

                virtual ~integrator();

                void integrate();

            private:
                struct priv;
                std::unique_ptr<priv> d_ptr;   // opaque pointer
            };
        }
    }
}
