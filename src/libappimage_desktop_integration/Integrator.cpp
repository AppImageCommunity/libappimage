// system
#include <cstdlib>
#include <sstream>
#include <utility>
#include <vector>

// libraries
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include "appimage/core/AppImage.h"
#include "appimage/core/Exceptions.h"
#include "utils/XdgUserDirs.h"
#include "utils/HashLib.h"
#include "ResourcesExtractor.h"
#include "appimage/desktop_integration/Integrator.h"

namespace bf = boost::filesystem;

using namespace appimage::core;
using namespace XdgUtils::DesktopEntry;

namespace appimage {
    namespace desktop_integration {
        /**
         * Implementation of the opaque pointer patter for the integrator class
         * see https://en.wikipedia.org/wiki/Opaque_pointer
         *
         * Contain a set of helper methods that will be used at the integrator class to fulfill the different task
         */
        struct Integrator::Priv {
            std::string userDataDir;
            std::string appImagePath;


            /**
             * @brief Build the file path were the AppImage desktop file should be copied in order to achieve the desktop
             * integration.
             *
             * The desktop file path is made by the following rule:
             * "$XDG_DATA_HOME/applications/<vendor id>_<uuid>-<application name scaped>.desktop"
             * where:
             *  - vendor id = appimagekit
             *  - uuid = appimage path md5 sum
             *  - application name scaped: the application name as in the Name entry at desktop file inside the AppImage
             *                              trime and with spaces replaced by underscores
             *
             * @param resources
             * @return desktop file path
             */
            std::string buildDesktopFilePath(const DesktopIntegrationResources& resources) {
                // Get AppImage path md5
                std::istringstream is(appImagePath);
                auto digest = utils::HashLib::md5(is);
                std::string pathMD5 = utils::HashLib::toHex(digest);

                std::stringstream desktopEntryStream;
                for (const auto& itr: resources)
                    if (itr.first.find(".desktop") != std::string::npos) {
                        desktopEntryStream = std::stringstream(std::string(itr.second.begin(), itr.second.end()));
                        break;
                    }

                const DesktopEntry desktopEntry(desktopEntryStream);

                // Get application name
                if (!desktopEntry.exists("Desktop Entry/Name"))
                    throw AppImageReadError("Error while reading AppImage desktop file. Missing Name entry.");

                // scape application name to make a valid desktop file name part
                std::string applicationNameScaped = desktopEntry.get("Desktop Entry/Name");
                boost::trim(applicationNameScaped);
                boost::replace_all(applicationNameScaped, " ", "_");

                // assemble the desktop file path
                std::string desktopFileName = "appimagekit_" + pathMD5 + "-" + applicationNameScaped + ".desktop";
                bf::path expectedDesktopFilePath(userDataDir + "/applications/" + desktopFileName);

                return expectedDesktopFilePath.string();
            }
        };

        Integrator::Integrator(const std::string& path) : priv(new Priv) {
            priv->appImagePath = path;
            priv->userDataDir = utils::XdgUserDirs::data();
        }

        Integrator::Integrator(const std::string& path, const std::string& xdgDataDir) : priv(new Priv) {
            priv->appImagePath = path;
            priv->userDataDir = xdgDataDir;
        }

        Integrator::~Integrator() = default;

        void Integrator::integrate() {
        }

        std::string Integrator::getDesktopFilePath() {
            ResourcesExtractor extractor(priv->appImagePath);
            auto resources = extractor.extract();
            return priv->buildDesktopFilePath(resources);
        }
    }
}
