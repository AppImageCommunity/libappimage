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
#include <appimage/core/AppImage.h>
#include <appimage/desktop_integration/exceptions.h>
#include <appimage/utils/ResourcesExtractor.h>
#include <constants.h>
#include "utils/Logger.h"
#include "utils/hashlib.h"
#include "utils/IconHandle.h"
#include "utils/path_utils.h"
#include "DesktopEntryEditor.h"
#include "Integrator.h"
#include "constants.h"

namespace bf = boost::filesystem;

using namespace appimage::core;
using namespace appimage::utils;
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

                ResourcesExtractor resourcesExtractor;
                DesktopEntry desktopEntry;

                Priv(const AppImage& appImage, const bf::path& xdgDataHome)
                    : appImage(appImage), xdgDataHome(xdgDataHome),
                      resourcesExtractor(appImage) {

                    if (xdgDataHome.empty())
                        throw DesktopIntegrationError("Invalid XDG_DATA_HOME: " + xdgDataHome.string());

                    // Extract desktop entry, DesktopIntegrationError will be throw if missing
                    auto desktopEntryPath = resourcesExtractor.getDesktopEntryPath();
                    auto desktopEntryData = resourcesExtractor.extractText(desktopEntryPath);
                    desktopEntry = std::move(DesktopEntry(desktopEntryData));

                    appImageId = hashPath(appImage.getPath());
                }

                /**
                 * Check if the AppImage author requested that this AppImage should not be integrated
                 */
                void assertItShouldBeIntegrated() {
                    try {
                        if (desktopEntry.exists("Desktop Entry/X-AppImage-Integrate")) {
                            const auto integrationRequested = static_cast<bool>(desktopEntry["Desktop Entry/X-AppImage-Integrate"]);

                            if (!integrationRequested)
                                throw DesktopIntegrationError("The AppImage explicitly requested to not be integrated");
                        }
                    } catch (const XdgUtils::DesktopEntry::BadCast& err) {
                        // if the value is not a bool we can ignore it
                        Logger::warning(err.what());
                    }
                }

                void deployDesktopEntry() {
                    bf::path desktopEntryDeployPath = buildDesktopFilePath();

                    // ensure that the parent path exists
                    create_directories(desktopEntryDeployPath.parent_path());

                    // update references to the deployed resources
                    DesktopEntry editedDesktopEntry = desktopEntry;
                    editDesktopEntry(editedDesktopEntry, appImageId);

                    // write file contents
                    std::ofstream desktopEntryFile(desktopEntryDeployPath.string());
                    desktopEntryFile << editedDesktopEntry;

                    // make it executable (required by some desktop environments)
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
                        VENDOR_PREFIX + "_" + appImageId + "-" + applicationNameScaped + ".desktop";
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

                    auto iconPaths = resourcesExtractor.getIconFilePaths(desktopEntryIconName);

                    // If the main app icon is not usr/share/icons we should deploy the .DirIcon in its place
                    if (iconPaths.empty()) {
                        Logger::warning(std::string("No icons found at \"") + iconsDirPath + "\"");

                        try {
                            Logger::warning("Using .DirIcon as default app icon");
                            auto dirIconData = resourcesExtractor.extract(dirIconPath);
                            deployApplicationIcon(desktopEntryIconName, dirIconData);;
                        } catch (const PayloadIteratorError& error) {
                            Logger::error(error.what());
                            Logger::error("No icon was generated for: " + appImage.getPath());
                        }
                    } else {
                        // Generate the target paths were the Desktop Entry icons will be deployed
                        std::map<std::string, std::string> iconFilesTargetPaths;
                        for (const auto& itr: iconPaths)
                            iconFilesTargetPaths[itr] = generateDeployPath(itr).string();

                        resourcesExtractor.extractTo(iconFilesTargetPaths);
                    }
                }

                /**
                 * Deploy <iconData> as the main application icon to
                 * XDG_DATA_HOME/icons/hicolor/<size>/apps/<vendorPrefix>_<appImageId>_<iconName>.<format extension>
                 *
                 * size: actual icon dimenzions, in case of vectorial image "scalable" is used
                 * format extension: in case of vectorial image "svg" otherwise "png"
                 *
                 * @param iconName
                 * @param iconData
                 */
                void deployApplicationIcon(const std::string& iconName, std::vector<char>& iconData) const {
                    try {
                        IconHandle icon(iconData);

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
                    } catch (const IconHandleError& er) {
                        Logger::error(er.what());
                        Logger::error("No icon was generated for: " + appImage.getPath());
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
                    fileNameBuilder << VENDOR_PREFIX << "_" << appImageId << "_" << path.filename().string();

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
                    auto mimeTypePackagesPaths = resourcesExtractor.getMimeTypePackagesPaths();
                    std::map<std::string, std::string> mimeTypePackagesTargetPaths;

                    // Generate deploy paths
                    for (const auto& path: mimeTypePackagesPaths)
                        mimeTypePackagesTargetPaths[path] = generateDeployPath(path).string();

                    resourcesExtractor.extractTo(mimeTypePackagesTargetPaths);
                }

                void setExecutionPermission() {
                    if (access(appImage.getPath().c_str(), X_OK) != F_OK)
                        try {
                            bf::permissions(appImage.getPath(), bf::owner_read | bf::owner_exe |
                                                                bf::group_read | bf::group_exe |
                                                                bf::others_read | bf::others_exe |
                                                                bf::add_perms);
                        } catch (const bf::filesystem_error&) {
                            Logger::error("Unable to set execution permissions on " + appImage.getPath());
                        }
                }
            };

            Integrator::Integrator(const AppImage& appImage, const bf::path& xdgDataHome)
                : d(new Priv(appImage, xdgDataHome)) {}

            Integrator::~Integrator() = default;

            void Integrator::integrate() {
                // an unedited desktop entry is required to identify the resources to be deployed
                d->assertItShouldBeIntegrated();

                // Must be executed before deployDesktopEntry because it changes the icon names
                d->deployIcons();
                d->deployDesktopEntry();
                d->deployMimeTypePackages();
                d->setExecutionPermission();
            }
        }
    }
}
