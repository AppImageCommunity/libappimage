// libraries
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

// local
#include "utils/Logger.h"
#include "MimeInfoEditor.h"

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            MimeInfoEditor::MimeInfoEditor(std::string data, std::string deployId)
                : data(std::move(data)), deployId(std::move(deployId)) {}

            std::string MimeInfoEditor::edit() {
                std::stringstream in(data), out;

                try {
                    using boost::property_tree::ptree;

                    // populate tree structure pt
                    ptree pt;
                    read_xml(in, pt);

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

                            std::string newIconName = originalName + "-" + deployId;

                            subTree.put("icon.<xmlattr>.name", newIconName);
                        }
                    }

                    write_xml(out, pt);
                } catch (const std::runtime_error& error) {
                    appimage::utils::Logger::warning(std::string("Unable to edit MimeInfo: ") + error.what());
                    return data;
                }

                return out.str();
            }
        }
    }
}
