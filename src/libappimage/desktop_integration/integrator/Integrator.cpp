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
#include "appimage/desktop_integration/exceptions.h"
#include "utils/HashLib.h"
#include "utils/IconHandle.h"
#include "utils/Logger.h"
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
            class Integrator::Priv {
            public:
                core::AppImage appImage;
                bf::path xdgDataHome;
                std::string appImageId;
                static const std::string vendorPrefix;

                DesktopEntry desktopEntry;
                DesktopIntegrationResources resources;
                utils::Logger logger;

                Priv(const AppImage& appImage, const std::string& xdgDataHome)
                    : appImage(appImage), xdgDataHome(xdgDataHome), logger("Integrator", std::clog) {

                    if (xdgDataHome.empty())
                        Priv::xdgDataHome = XdgUtils::BaseDir::XdgDataHome();
                }

                /**
                 * The AppImage Id is build from the MD5 hash sum of it's canonical path.
                 */
                void buildAppImageId() {
                    auto res = appimage_get_md5(appImage.getPath().c_str());
                    if (res != nullptr)
                        appImageId = res;
                    else
                        appImageId = "";

                    free(res);
                }

                /**
                 * Extract from the AppImage Payload the resources required to properly integrate it
                 * with the desktop environment.
                 */
                void readResources() {
                    try {
                        ResourcesExtractor extractor(appImage);
                        /**
                         * The following resources are required:
                         * - desktop entry (mandatory)
                         * - icons (optional)
                         * - AppStrean medatada (optional)
                         * - mime-type descriptors (optional)
                         */
                        resources = extractor.extract();

                        // a desktop entry is required for the integration process
                        if (resources.desktopEntryPath.empty())
                            throw DesktopIntegrationError("Missing Desktop File");

                    } catch (const AppImageError& error) {
                        throw DesktopIntegrationError(
                            std::string("desktop integration resources failed: ") + error.what());
                    }
                }

                void loadDesktopEntry() {
                    // prepare a istream to fill the DesktopEntry
                    std::string desktopEntryData(resources.desktopEntryData.begin(),
                                                 resources.desktopEntryData.end());

                    std::stringstream desktopEntryDataStream(desktopEntryData);

                    // load the desktop entry data into a DesktopEntry class to ease edition
                    desktopEntryDataStream >> desktopEntry;

                    // Check if the AppImage author requested that this AppImage should not be integrated
                    auto integrationRequest = desktopEntry.get("Desktop Entry/X-AppImage-Integrate");
                    boost::algorithm::erase_all(integrationRequest, " ");
                    boost::algorithm::to_lower(integrationRequest);
                    if (integrationRequest == "false")
                        throw DesktopIntegrationError("The AppImage explicitly requested to not be integrated");
                }

                void deployDesktopEntry() {
                    // update references to the deployed resources
                    editDesktopEntry(desktopEntry, appImageId);

                    bf::path desktopEntryDeployPath = buildDesktopFilePath();

                    // ensure that the parent path exists
                    create_directories(desktopEntryDeployPath.parent_path());

                    // write file contents
                    std::ofstream desktopEntryFile(desktopEntryDeployPath.string());
                    desktopEntryFile << desktopEntry;

                    // make it executable
                    bf::permissions(desktopEntryDeployPath, bf::owner_read | bf::owner_exe | bf::add_perms);
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
                    if (!desktopEntry.exists("Desktop Entry/Name"))
                        throw IOError("Error while reading AppImage desktop file. Missing Name entry.");

                    // scape application name to make a valid desktop file name part
                    std::string applicationNameScaped = desktopEntry.get("Desktop Entry/Name");
                    boost::trim(applicationNameScaped);
                    boost::replace_all(applicationNameScaped, " ", "_");

                    // assemble the desktop file path
                    std::string desktopFileName =
                        vendorPrefix + "_" + appImageId + "-" + applicationNameScaped + ".desktop";
                    bf::path expectedDesktopFilePath(xdgDataHome / "applications" / desktopFileName);

                    return expectedDesktopFilePath.string();
                }

                /**
                 * Set the desktop entry paths to their expected locations
                 * @param entry
                 * @param md5str
                 */
                void editDesktopEntry(DesktopEntry& entry, const std::string& md5str) const {
                    // Prepare Desktop Entry editor
                    DesktopEntryEditor editor;
                    // Set the path used in the Exec and tryExec fields
                    editor.setAppImagePath(appImage.getPath());
                    // Set the identifier to be used while prefixing the icon files
                    editor.setIdentifier(md5str);
                    // Apply changes to the desktop entry
                    editor.edit(entry);
                }

                /**
                 * Find and deploy the AppImage icons resources.
                 * Icons at usr/share/icons will be preferred if not available the ".DirIcon" will be used.
                 */
                void deployIcons() {
                    static const std::string dirIconPath = ".DirIcon";
                    static const auto iconsDirPath = "usr/share/icons";

                    // get the name of the icon used in the desktop entry
                    const std::string desktopEntryIconName = desktopEntry.get("Desktop Entry/Icon");
                    if (desktopEntryIconName.empty())
                        throw DesktopIntegrationError("Missing icon field in the desktop entry");

                    // icons at usr/share/icons are preferred otherwise fallback to ".DirIcon"
                    bool useDirIcon = true;
                    for (const auto& itr: resources.icons) {
                        if (itr.first.find(iconsDirPath) != std::string::npos &&
                            itr.first.find(desktopEntryIconName) != std::string::npos) {
                            useDirIcon = false;
                            break;
                        }
                    }

                    // If the main app icon is not usr/share/icons we should deploy the .DirIcon in its place
                    if (useDirIcon) {
                        logger.warning() << "No icons found at \"" << iconsDirPath << "\"" << std::endl;

                        // ".DirIcon" exists
                        auto ptr = resources.icons.find(".DirIcon");
                        if (ptr != resources.icons.end() && !ptr->second.empty()) {
                            logger.warning() << "Using .DirIcon as default app icon" << std::endl;
                            std::vector<uint8_t> iconData{ptr->second.begin(), ptr->second.end()};

                            deployApplicationIcon(desktopEntryIconName, iconData);
                        } else {
                            logger.warning() << ".DirIcon wans't found or not extracted" << std::endl;
                            logger.error() << "No icon was generated for: " << appImage.getPath() << std::endl;
                        }
                    } else {
                        for (const auto& itr: resources.icons) {
                            // only deploy icons in "usr/share/icons"
                            if (itr.first.find(iconsDirPath) != std::string::npos) {
                                auto deployPath = generateDeployPath(itr.first);
                                auto& fileData = itr.second;

                                // create parent dirs
                                bf::create_directories(deployPath.parent_path());

                                // write file contents
                                bf::ofstream file(deployPath);
                                file.write(fileData.data(), fileData.size());
                                file.close();
                            }
                        }
                    }
                }

                /**
                 * Deploy application icon to
                 * XDG_DATA_HOME/icons/hicolor/<size>/apps/<vendorPrefix>_<appImageId>_<iconName>.<format extension>
                 *
                 * size: actual icon dimenzions, in case of vectorial image "scalable" is used
                 * format extension: in case of vectorial image "svg" otherwise "png"
                 *
                 * @param iconName
                 * @param iconData
                 */
                void deployApplicationIcon(const std::string& iconName, std::vector<uint8_t>& iconData) const {
                    try {
                        utils::IconHandle icon(iconData);

                        // build the icon path and name attending to its format and size as
                        // icons/hicolor/<size>/apps/<vendorPrefix>_<appImageId>_<iconName>.<format extension>
                        boost::filesystem::path iconPath = "icons/hicolor";
                        std::stringstream iconNameBuilder;
                        iconNameBuilder << iconName;

                        // in case of vectorial images use ".svg" as extension and "scalable" as size
                        if (icon.format() == "svg") {
                            iconNameBuilder << ".svg";
                            iconPath /= "scalable";
                        } else {
                            // otherwise use "png" as extension and the actual icon size as size
                            iconNameBuilder << ".png";

                            auto iconSize = std::to_string(icon.getSize());
                            iconPath /= (iconSize + "x" + iconSize);
                        }

                        iconPath /= "apps";
                        iconPath /= iconNameBuilder.str();

                        auto deployPath = generateDeployPath(iconPath);
                        icon.save(deployPath.string(), icon.format());
                    } catch (const utils::IconHandleError& er) {
                        logger.error() << er.what() << std::endl;
                        logger.error() << "No icon was generated for: " << appImage.getPath() << std::endl;
                    }
                }

                /**
                 * Append vendor prefix and appImage id to the file names to identify the appImage that owns
                 * this file. Replace the default XDG_DATA_DIR by the one at <xdgDataHome>
                 *
                 * @param path resource path
                 * @return path with a prefixed file name
                 */
                bf::path generateDeployPath(bf::path path) const {
                    // add appImage resource identification prefix to the filename
                    std::stringstream fileNameBuilder;
                    fileNameBuilder << vendorPrefix << "_" << appImageId << "_" << path.filename().string();

                    // build the relative parent path ignoring the default XDG_DATA_DIR prefix ("usr/share")
                    path.remove_filename();
                    bf::path relativeParentPath;
                    const bf::path defaultXdgDataDirPath = "usr/share";

                    for (const auto& itr : path) {
                        relativeParentPath /= itr;

                        if (relativeParentPath == defaultXdgDataDirPath)
                            relativeParentPath.clear();
                    }

                    bf::path newPath = xdgDataHome / relativeParentPath / fileNameBuilder.str();
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
                    bf::permissions(appImage.getPath(), bf::owner_read | bf::owner_exe | bf::add_perms);
                }
            };

            // in-line initialization is not allowed for static values
            const std::string Integrator::Priv::vendorPrefix = "appimagekit";

            Integrator::Integrator(const AppImage& appImage, const std::string& xdgDataHome)
                : priv(new Priv(appImage, xdgDataHome)) {}

            Integrator::~Integrator() = default;

            void Integrator::integrate() {
                // Extract and prepare the desktop integration resources
                priv->readResources();
                priv->loadDesktopEntry();
                priv->buildAppImageId();

                // Must be executed before deployDesktopEntry because it changes the icon names
                priv->deployIcons();
                priv->deployDesktopEntry();
                priv->deployMimeTypePackages();
                priv->setExecutionPermission();
            }
        }
    }
}
