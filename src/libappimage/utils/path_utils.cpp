//system
#include <string>

// libraries
#include <boost/filesystem.hpp>

// local
#include "path_utils.h"
#include "HashLib.h"

namespace appimage {
    namespace utils {
        std::string pathToURI(const std::string& path) {
            if (path.compare(0, 7, "file://") != 0)
                return "file://" + path;
            else
                return path;
        }

        std::string hashPath(const boost::filesystem::path& path) {
            if (path.empty())
                return {};

            const auto& canonicalPath = boost::filesystem::weakly_canonical(path);

            if (canonicalPath.empty())
                return {};

            auto uri = pathToURI(canonicalPath.string());

            const auto md5raw = HashLib::md5(uri);
            const auto md5Str = HashLib::toHex(md5raw);

            return md5Str;
        }
    }
}
