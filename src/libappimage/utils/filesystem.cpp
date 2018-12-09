#include <sstream>

extern "C" {
#include <sys/stat.h>
#include <unistd.h>
}

#include "core/exceptions.h"
#include "filesystem.h"

using namespace appimage::utils::filesystem;

std::string appimage::utils::filesystem::parentPath(const std::string& path) {
    long i = path.length() - 1;
    while (i >= 0 && path[i] != '/')
        i--;

    while (i > 0 && path[i - 1] == '/')
        i--;


    if (i >= 0)
        return path.substr(0, i);
    else
        return path;

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
