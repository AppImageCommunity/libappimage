#pragma once

// system
#include <string>

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            /**
             * @brief Modifies MimeInfo files to be deployed.
             *
             *
             */
            class MimeInfoEditor {
            public:
                /**
                 * Create an editor instance to modify the
                 * @param data
                 * @param deployId
                 */
                MimeInfoEditor(std::string data, std::string deployId);

                std::string edit();

            private:
                std::string data;
                std::string deployId;
            };
        }
    }
}
