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
             * @brief Edit MimeInfo files to be deployed
             */
            class MimeInfoEditor {
            public:
                /**
                 * Create an editor instance to modify the
                 * @param data
                 * @param deployId
                 */
                MimeInfoEditor(std::string data);

                /**
                 * Set the deploy id to be prepended to the icon file name
                 * @param deployId
                 */
                void setDeployId(const std::string& deployId);

                /**
                 * Apply modification to the MimeInfo file
                 * @return modified MimeInfo
                 */
                std::string edit();

                /**
                 * Extract the icon names from from the icon entry or the mime type name
                 * @return names of the icons related to the mime types
                 */
                std::list<std::string> getMimeTypeIconNames() const;

            private:
                boost::property_tree::ptree pt;
                std::string deployId;
            };
        }
    }
}
