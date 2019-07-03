// system
#include <iostream>

// libraries
#include <boost/program_options.hpp>

#include <appimage/core/AppImage.h>
#include <appimage/utils/logging.h>
#include <appimage/desktop_integration/IntegrationManager.h>

namespace po = boost::program_options;

void unregisterAppImage(bool verbose, const appimage::desktop_integration::IntegrationManager& integrationManager,
                        const std::string& appImagePath) {
    if (verbose)
        std::cout << "Un-registering " << appImagePath << std::endl;
    integrationManager.unregisterAppImage(appImagePath);
}

void registerAppImage(bool verbose, const appimage::desktop_integration::IntegrationManager& integrationManager,
                      const std::string& appImagePath) {
    if (verbose)
        std::cout << "Registering " << appImagePath << std::endl;

    appimage::core::AppImage appImage(appImagePath);
    integrationManager.registerAppImage(appImage);
}

int main(int argc, char** argv) {
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("verbose", "show verbose output")
        ("register", "register the <appimages> on the system")
        ("unregister", "unregister the <appimages> frpom the system")
        ("appimage", po::value<std::vector<std::string> >(), "appimage");

    po::positional_options_description p;
    p.add("appimage", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    bool verbose = vm.count("verbose");

    if (vm.count("appimage")) {
        appimage::desktop_integration::IntegrationManager integrationManager;

        for (const std::string& appImagePath : vm["appimage"].as<std::vector<std::string> >()) {
            try {
                if (vm.count("register"))
                    registerAppImage(verbose, integrationManager, appImagePath);

                if (vm.count("unregister"))
                    unregisterAppImage(verbose, integrationManager, appImagePath);

            } catch (const std::runtime_error& error) {
                std::cout << error.what() << std::endl;
            }
        }
    } else {
        std::cout << "Missing target appimage files" << std::endl;
        std::cout << desc << std::endl;
        return 1;
    }

    return 0;
}



