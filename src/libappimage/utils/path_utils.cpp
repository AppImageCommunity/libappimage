//system
#include <string>

// libraries
#include <boost/filesystem.hpp>

// local
#include "path_utils.h"
#include "hashlib.h"

namespace bf = boost::filesystem;

namespace appimage {
    namespace utils {
        std::string pathToURI(const std::string& path) {
            if (path.compare(0, 7, "file://") != 0)
                return "file://" + path;
            else
                return path;
        }

        std::string hashPath(const bf::path& path) {
            if (path.empty())
                return {};

            const auto& canonicalPath = bf::absolute(path);

            if (canonicalPath.empty())
                return {};

            auto uri = pathToURI(canonicalPath.string());

            const auto md5raw = hashlib::md5(uri);
            const auto md5Str = hashlib::toHex(md5raw);

            return md5Str;
        }
    }
}
