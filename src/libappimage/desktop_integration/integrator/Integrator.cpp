// system
#include <cstdlib>
#include <sstream>
#include <utility>
#include <vector>
#include <iostream>
#include <fstream>

// libraries
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>
#include <XdgUtils/BaseDir/BaseDir.h>

// local
#include "appimage/appimage.h"
#include "appimage/core/AppImage.h"
#include "appimage/desktop_integration/Exceptions.h"
#include "utils/HashLib.h"
#include "utils/IconHandle.h"
#include "DesktopEntryEditor.h"
#include "ResourcesExtractor.h"
#include "Integrator.h"

namespace bf = boost::filesystem;

using namespace appimage::core;
using namespace XdgUtils::DesktopEntry;

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            /**
             * Implementation of the opaque pointer patter for the integrator class
             * see https://en.wikipedia.org/wiki/Opaque_pointer
             *
             * Contain a set of helper methods that will be used at the integrator class to fulfill the different task
             */
            struct Integrator::Priv {
                bf::path xdgDataHome;
                bf::path appImagePath;
                std::string appImageId;
                std::string vendorPrefix = "appimagekit";

                DesktopEntry entry;
                DesktopIntegrationResources resources;

                /**
                 * The AppImage Id is build from the MD5 hash sum of it's path.
                 */
                void buildAppImageId() {
                    appImageId = appimage_get_md5(appImagePath.string().c_str()) ?: "";
                }

                void readResources() {
                    try {
                        ResourcesExtractor extractor(appImagePath.string());

                        resources = extractor.extract();
                        if (resources.desktopEntryPath.empty())
                            throw DesktopIntegrationError("Missing Desktop File");

                    } catch (const AppImageError& error) {
                        throw DesktopIntegrationError("Unable to extract the desktop integration resources.");
                    }
                }

                void loadDesktopEntry() {
                    std::string desktopEntryData(resources.desktopEntryData.begin(),
                                                 resources.desktopEntryData.end());

                    std::stringstream desktopEntryDataStream(desktopEntryData);

                    desktopEntryDataStream >> entry;

                    // Check if the AppImage author requested that this AppImage should not be integrated
                    auto integrationRequest = entry.get("Desktop Entry/X-AppImage-Integrate");
                    boost::algorithm::erase_all(integrationRequest, " ");
                    boost::algorithm::to_lower(integrationRequest);
                    if (integrationRequest == "false")
                        throw DesktopIntegrationError("The AppImage explicitly requested to not be integrated");
                }

                void deployDesktopEntry() {
                    editDesktopEntry(entry, appImageId);

                    // deploy desktop entry
                    bf::path desktopEntryDeployPath = buildDesktopFilePath();

                    // create dir
                    create_directories(desktopEntryDeployPath.parent_path());

                    // write file contents
                    std::ofstream desktopEntryFile(desktopEntryDeployPath.string());
                    desktopEntryFile << entry;

                    // make it executable
                    bf::permissions(desktopEntryDeployPath, bf::owner_read | bf::owner_exe);
                }

                /**
                 * @brief Build the file path were the AppImage desktop file should be copied in order to achieve the desktop
                 * integration.
                 *
                 * The desktop file path is made by the following rule:
                 * "$XDG_DATA_HOME/applications/<vendor id>_<appImageId>-<application name scaped>.desktop"
                 * where:
                 *  - vendor id = appimagekit
                 *  - appImageId = appimage path md5 sum
                 *  - application name escaped: the application name as in the Name entry at desktop file inside the
                 *                              AppImage with spaces replaced by underscores
                 *
                 * @param resources
                 * @return desktop file path
                 */
                std::string buildDesktopFilePath() const {
                    // Get application name
                    if (!entry.exists("Desktop Entry/Name"))
                        throw AppImageReadError("Error while reading AppImage desktop file. Missing Name entry.");

                    // scape application name to make a valid desktop file name part
                    std::string applicationNameScaped = entry.get("Desktop Entry/Name");
                    boost::trim(applicationNameScaped);
                    boost::replace_all(applicationNameScaped, " ", "_");

                    // assemble the desktop file path
                    std::string desktopFileName =
                        vendorPrefix + "_" + appImageId + "-" + applicationNameScaped + ".desktop";
                    bf::path expectedDesktopFilePath(xdgDataHome / "applications" / desktopFileName);

                    return expectedDesktopFilePath.string();
                }

                void editDesktopEntry(DesktopEntry& entry, const std::string& md5str) const {
                    // Modify the Desktop Entry
                    DesktopEntryEditor editor;
                    editor.setAppImagePath(appImagePath.string());
                    editor.setIdentifier(md5str);

                    editor.edit(entry);
                }

                void deployIcons() {
                    const std::string dirIconPath = ".DirIcon";
                    std::map<std::string, bf::path> deployPaths;

                    // Generate deploy paths
                    for (const auto& itr: resources.icons) {
                        if (itr.first.find("usr/share/icons") != std::string::npos)
                            deployPaths[itr.first] = generateDeployPath(itr.first);
                    }

                    // If the main app icon is not usr/share/icons we should deploy the .DirIcon in its place
                    if (deployPaths.empty()) {
                        std::clog << "WARNING: No icons found at \"usr/share/icons\"" << std::endl;


                        auto ptr = resources.icons.find(".DirIcon");

                        if (ptr != resources.icons.end() && !ptr->second.empty()) {
                            std::clog << "WARNING: Using .DirIcon as default app icon" << std::endl;

                            try {
                                std::vector<uint8_t> iconData{ptr->second.begin(), ptr->second.end()};
                                utils::IconHandle icon(iconData);
                                bf::path iconPath = xdgDataHome / "icons/hicolor";


                                std::stringstream iconName;
                                iconName << entry.get("Desktop Entry/Icon");
                                if (icon.format() == "svg") {
                                    iconName << ".svg";
                                    iconPath /= "scalable";
                                } else {
                                    iconName << ".png";

                                    auto iconSize = std::to_string(icon.getSize());
                                    iconPath /= (iconSize + "x" + iconSize);
                                }

                                iconPath /= "apps";
                                iconPath /= iconName.str();

                                icon.save(iconPath.string(), icon.format());
                            } catch (const utils::IconHandleError& er) {
                                std::clog << "ERROR: " << er.what() << std::endl;
                                std::clog << "ERROR: No icon was generated for: " << appImagePath << std::endl;
                            }
                        } else {
                            std::clog << "WARNING: .DirIcon wans't found or not extracted" << std::endl;
                            std::clog << "ERROR: No icon was generated for: " << appImagePath << std::endl;
                        }
                    } else {
                        for (const auto& itr: deployPaths) {
                            auto& fileData = resources.icons[itr.first];

                            // create parent dirs
                            bf::create_directories(itr.second.parent_path());

                            // write file contents
                            std::ofstream file(itr.second.string());
                            file.write(fileData.data(), fileData.size());
                            file.close();
                        }
                    }
                }


                bf::path generateDeployPath(const std::string& path) const {
                    boost::filesystem::path oldPath(path.substr(10));

                    std::stringstream iconFileName;
                    iconFileName << vendorPrefix << "_" << appImageId << "_" << oldPath.filename().string();

                    boost::filesystem::path newPath = xdgDataHome / oldPath.parent_path() / iconFileName.str();
                    return newPath;
                }

                void deployMimeTypePackages() {
                    // Generate deploy paths
                    for (const auto& itr: resources.mimeTypePackages) {
                        auto& fileData = itr.second;
                        auto deployPath = generateDeployPath(itr.first);

                        // create parent dirs
                        bf::create_directories(deployPath);

                        // write file contents
                        std::ofstream file(deployPath.string());
                        file.write(fileData.data(), fileData.size());
                        file.close();
                    }
                }

                void setExecutionPermission() {
                    bf::permissions(appImagePath, bf::owner_read | bf::owner_exe);
                }
            };


            Integrator::Integrator(const std::string& path, const std::string& xdgDataHome) : priv(new Priv) {
                priv->appImagePath = path;
                if (xdgDataHome.empty())
                    priv->xdgDataHome = XdgUtils::BaseDir::XdgDataHome();
                else
                    priv->xdgDataHome = xdgDataHome;
            }

            Integrator::~Integrator() = default;

            void Integrator::integrate() {
                priv->buildAppImageId();
                priv->readResources();
                priv->loadDesktopEntry();

                priv->deployDesktopEntry();
                priv->deployIcons();
                priv->deployMimeTypePackages();
                priv->setExecutionPermission();
            }
        }
    }
}
