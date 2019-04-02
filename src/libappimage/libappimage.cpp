/**
 * Implementation of the C interface functions
 */
// system
#include <cstring>
#include <sstream>

//for std::underlying_type
#include <type_traits>

// libraries
extern "C" {
#include <glob.h>
}
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <XdgUtils/BaseDir/BaseDir.h>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include <appimage/utils/ResourcesExtractor.h>
#include <appimage/core/AppImage.h>
#include "utils/Logger.h"
#include "utils/hashlib.h"
#include "utils/UrlEncoder.h"
#include "utils/path_utils.h"

#ifdef LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED

#include <appimage/desktop_integration/IntegrationManager.h>
#include <appimage/appimage.h>
#endif

using namespace appimage::core;
using namespace appimage::utils;

namespace bf = boost::filesystem;

/**
 * We cannot allow any exception to scape from C++ to C. This will wrap a given code block into a try-catch
 * structure and will effectively catch and log all exceptions.
 *
 * @param src
 */
#define CATCH_ALL(src) { \
    try { \
        src \
    }  catch (const std::runtime_error& err) { \
        Logger::error(std::string(__FUNCTION__) + " : " + err.what()); \
    } catch (...) { \
        Logger::error(std::string(__FUNCTION__) + " : " + " unexpected error"); \
    } \
}

extern "C" {


/* Check if a file is an AppImage. Returns the image type if it is, or -1 if it isn't */
int appimage_get_type(const char* path, bool) {
    typedef std::underlying_type<AppImageFormat>::type utype;
    CATCH_ALL(
        AppImage appImage(path);
        return static_cast<utype>(appImage.getFormat());
    );

    return static_cast<utype>(AppImageFormat::INVALID);
}

char** appimage_list_files(const char* path) {
    char** result = nullptr;
    CATCH_ALL(
        AppImage appImage(path);

        std::vector<std::string> files;
        for (auto itr = appImage.files(); itr != itr.end(); ++itr)
            if (!(*itr).empty())
                files.emplace_back(*itr);


        result = static_cast<char**>(malloc(sizeof(char*) * (files.size() + 1)));
        for (int i = 0; i < files.size(); i++)
            result[i] = strdup(files[i].c_str());

        result[files.size()] = nullptr;

        return result;
    );

    // Create empty string list
    result = static_cast<char**>(malloc(sizeof(char*)));
    result[0] = nullptr;

    return result;
}

void appimage_string_list_free(char** list) {
    for (char** ptr = list; ptr != NULL && *ptr != NULL; ptr++)
        free(*ptr);

    free(list);
}

bool
appimage_read_file_into_buffer_following_symlinks(const char* appimage_file_path, const char* file_path, char** buffer,
                                                  unsigned long* buf_size) {
    // Init output params
    *buffer = nullptr;
    *buf_size = 0;

    CATCH_ALL(
        AppImage appImage(appimage_file_path);
        appimage::utils::ResourcesExtractor resourcesExtractor(appImage);

        auto fileData = resourcesExtractor.extract(file_path);

        *buffer = static_cast<char*>(malloc(sizeof(char) * fileData.size()));
        std::copy(fileData.begin(), fileData.end(), *buffer);

        *buf_size = fileData.size();

        return true;
    );

    return false;
}

void appimage_extract_file_following_symlinks(const char* appimage_file_path, const char* file_path,
                                              const char* target_file_path) {
    CATCH_ALL(
        AppImage appImage(appimage_file_path);
        appimage::utils::ResourcesExtractor resourcesExtractor(appImage);

        resourcesExtractor.extractTo({{file_path, target_file_path}});
    );
}


/*
 * Checks whether an AppImage's desktop file has set X-AppImage-Integrate=false.
 * Useful to check whether the author of an AppImage doesn't want it to be integrated.
 *
 * Returns >0 if set, 0 if not set, <0 on errors.
 */
int appimage_shall_not_be_integrated(const char* path) {
    CATCH_ALL(
        AppImage appImage(path);
        XdgUtils::DesktopEntry::DesktopEntry entry;
        // Load Desktop Entry
        for (auto itr = appImage.files(); itr != itr.end(); ++itr) {
            const auto& entryPath = *itr;
            if (entryPath.find(".desktop") != std::string::npos && entryPath.find('/') == std::string::npos) {
                itr.read() >> entry;
                break;
            }
        }

        auto integrateEntryValue = entry.get("Desktop Entry/X-AppImage-Integrate", "true");

        boost::to_lower(integrateEntryValue);
        boost::algorithm::trim(integrateEntryValue);

        return integrateEntryValue == "false";
    );

    return -1;
}


/*
 * Checks whether an AppImage's desktop file has set Terminal=true.
 * Useful to check whether the author of an AppImage doesn't want it to be integrated.
 *
 * Returns >0 if set, 0 if not set, <0 on errors.
 */
int appimage_is_terminal_app(const char* path) {
    CATCH_ALL(
        AppImage appImage(path);

        std::vector<char> data;

        XdgUtils::DesktopEntry::DesktopEntry entry;
        // Load Desktop Entry
        for (auto itr = appImage.files(); itr != itr.end(); ++itr) {
            const auto& entryPath = *itr;
            if (entryPath.find(".desktop") != std::string::npos && entryPath.find("/") == std::string::npos) {
                itr.read() >> entry;
                break;
            }
        }

        auto terminalEntryValue = entry.get("Desktop Entry/Terminal");

        boost::to_lower(terminalEntryValue);
        boost::algorithm::trim(terminalEntryValue);

        return terminalEntryValue == "true";
    );

    return -1;
}


/* Return the md5 hash constructed according to
 * https://specifications.freedesktop.org/thumbnail-spec/thumbnail-spec-latest.html#THUMBSAVE
 * This can be used to identify files that are related to a given AppImage at a given location */
char* appimage_get_md5(const char* path) {
    using namespace appimage::utils;

    if (path == nullptr)
        return nullptr;

    CATCH_ALL(
        auto hash = hashPath(path);
        if (hash.empty())
            return nullptr;
        else
            return strdup(hash.c_str());
    );

    return nullptr;
}


off_t appimage_get_payload_offset(char const* path) {
    if (path == nullptr)
        return 0;

    CATCH_ALL(
        return AppImage(path).getPayloadOffset();
    );

    return 0;
}


char* appimage_registered_desktop_file_path(const char* path, char* md5, bool verbose) {
    glob_t pglob = {};

    // if md5 has been calculated before, we can just use it to save these extra calculations
    // if not, we need to calculate it here
    if (md5 == nullptr)
        md5 = appimage_get_md5(path);

    // sanity check
    if (md5 == nullptr) {
        if (verbose)
            fprintf(stderr, "appimage_get_md5() failed\n");
        return nullptr;
    }

    std::string data_home = XdgUtils::BaseDir::XdgDataHome();

    // TODO: calculate this value exactly
    char* glob_pattern = static_cast<char*>(malloc(PATH_MAX));
    sprintf(glob_pattern, "%s/applications/appimagekit_%s-*.desktop", data_home.c_str(), md5);

    glob(glob_pattern, 0, nullptr, &pglob);

    char* rv = nullptr;

    if (pglob.gl_pathc <= 0) {
        if (verbose) {
            fprintf(stderr, "No results found by glob()");
        }
    } else if (pglob.gl_pathc >= 1) {
        if (pglob.gl_pathc > 1 && verbose) {
            fprintf(stderr, "Too many results returned by glob(), returning first result found");
        }

        // need to copy value to be able to globfree() later on
        rv = strdup(pglob.gl_pathv[0]);
    }

    globfree(&pglob);

    return rv;
}

#ifdef LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED
using namespace appimage::desktop_integration;

/*
 * Register an AppImage in the system
 * Returns 0 on success, non-0 otherwise.
 */
int appimage_register_in_system(const char* path, bool verbose) {
    CATCH_ALL(
        AppImage appImage(path);
        IntegrationManager manager;
        manager.registerAppImage(appImage);

#ifdef LIBAPPIMAGE_THUMBNAILER_ENABLED
        manager.generateThumbnails(appImage);
#endif // LIBAPPIMAGE_THUMBNAILER_ENABLED
        return 0;
    );

    return 1;
}


/* Unregister an AppImage in the system */
int appimage_unregister_in_system(const char* path, bool verbose) {
    if (path == nullptr)
        return 1;

    CATCH_ALL(
        IntegrationManager manager;
        manager.unregisterAppImage(path);

#ifdef LIBAPPIMAGE_THUMBNAILER_ENABLED
        manager.removeThumbnails(path);
#endif // LIBAPPIMAGE_THUMBNAILER_ENABLED
        return 0;
    );

    return 1;
}

/* Check whether AppImage is registered in the system already */
bool appimage_is_registered_in_system(const char* path) {
    if (path == nullptr)
        return false;

    CATCH_ALL(
        IntegrationManager manager;
        return manager.isARegisteredAppImage(path);
    );

    return false;
}


#ifdef LIBAPPIMAGE_THUMBNAILER_ENABLED
/* Create AppImage thumbanil according to
 * https://specifications.freedesktop.org/thumbnail-spec/0.8.0/index.html
 */
bool appimage_create_thumbnail(const char* appimage_file_path, bool verbose) {
    CATCH_ALL(
        AppImage appImage(appimage_file_path);
        IntegrationManager manager;
        manager.generateThumbnails(appImage);
        return true;
    );

    return false;
}

#endif // LIBAPPIMAGE_THUMBNAILER_ENABLED
#endif // LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED

}
