// system
#include <sstream>

extern "C" {
#include <sys/stat.h>
#include <unistd.h>
}

#include "core/exceptions.h"
#include "filesystem.h"

using namespace appimage::utils::filesystem;

std::string appimage::utils::filesystem::parentPath(const std::string& path) {
    // find last directory separator
    long i = path.rfind('/');

    // directory separators found
    if (i == std::string::npos)
        return ".";

    // remove repeated separators
    while (i > 0 && path[i - 1] == '/')
        i--;

    // if the only '/' is at the beginning of the string take it as the filesystem root
    if (i == 0)
        return "/";

    // assume that the parent path was found if still not in the end of the string
    if (i > 0)
        return path.substr(0, i);

    return ".";
}

void appimage::utils::filesystem::createDirectories(const std::string& path) {
    for (int i = 0; i < path.length(); i++) {
        if (path[i] == '/') {
            auto subDir = path.substr(0, i + 1);
            if (access(subDir.c_str(), F_OK) != 0) {
                if (mkdir(subDir.c_str(), 0777) == -1)
                    throw core::AppImageError("mkdir error " + path);
            }
        }
    }

    if (access(path.c_str(), F_OK) != 0) {
        if (mkdir(path.c_str(), 0777) == -1)
            throw core::AppImageError("mkdir error " + path);
    }
}
