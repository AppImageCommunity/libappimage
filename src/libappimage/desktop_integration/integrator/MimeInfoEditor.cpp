// libraries
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <appimage_handler.h>

// local
#include "utils/Logger.h"
#include "MimeInfoEditor.h"

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            MimeInfoEditor::MimeInfoEditor(std::string data) {
                try {
                    std::stringstream in(data);
                    using boost::property_tree::ptree;

                    // populate tree structure pt
                    read_xml(in, pt);
                } catch (const std::runtime_error& error) {
                    appimage::utils::Logger::warning(std::string("Unable to read MimeInfo: ") + error.what());
                }
            }

            std::string MimeInfoEditor::edit() {
                try {
                    using boost::property_tree::ptree;

                    // traverse pt
                    for (auto& node:  pt.get_child("mime-info")) {
                        if (node.first == "mime-type") {
                            auto& subTree = node.second;
                            // get original icon name from the icon entry
                            std::string originalName = subTree.get<std::string>("icon.<xmlattr>.name", "");

                            // or fallback to the mime-type name
                            if (originalName.empty()) {
                                originalName = subTree.get<std::string>("<xmlattr>.type");
                                boost::replace_all(originalName, "/", "-");
                            }

                            std::string newIconName = deployId + '_' + originalName;

                            subTree.put("icon.<xmlattr>.name", newIconName);
                        }
                    }

                    std::stringstream out;
                    write_xml(out, pt);
                    return out.str();
                } catch (const std::runtime_error& error) {
                    appimage::utils::Logger::warning(std::string("Unable to edit MimeInfo: ") + error.what());
                    return std::string{};
                }
            }

            void MimeInfoEditor::setDeployId(const std::string& deployId) {
                MimeInfoEditor::deployId = deployId;
            }

            std::list<std::string> MimeInfoEditor::getMimeTypeIconNames() const {
                std::list<std::string> icons;

                try {
                    using boost::property_tree::ptree;

                    // traverse pt
                    for (auto& node:  pt.get_child("mime-info")) {
                        if (node.first == "mime-type") {
                            auto& subTree = node.second;
                            // get original icon name from the icon entry
                            std::string iconName = subTree.get<std::string>("icon.<xmlattr>.name", "");

                            // or fallback to the mime-type name
                            if (iconName.empty()) {
                                iconName = subTree.get<std::string>("<xmlattr>.type");
                                boost::replace_all(iconName, "/", "-");
                            }

                            icons.push_back(iconName);
                        }
                    }
                } catch (const std::runtime_error& error) {
                    appimage::utils::Logger::warning(std::string("Unable to read MimeInfo: ") + error.what());
                }

                return icons;
            }
        }
    }
}
