#pragma once

// system
#include <list>
#include <string>

// libraries
#include <boost/property_tree/ptree.hpp>

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
                MimeInfoEditor(std::string data);

                void setDeployId(const std::string& deployId);

                std::string edit();

                std::list<std::string> getMimeTypeIconNames();

            private:
                boost::property_tree::ptree pt;
                std::string deployId;
            };
        }
    }
}
