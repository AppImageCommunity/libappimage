// system
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>

// local
#include "TemporaryDirectory.h"

TemporaryDirectory::TemporaryDirectory(const std::string& prefix) {
    std::filesystem::path tmpl;

    const auto TEMPDIR = getenv("TEMPDIR");

    if (TEMPDIR != nullptr) {
        tmpl = TEMPDIR;
    } else {
        tmpl = std::filesystem::temp_directory_path();
    }

    tmpl /= "libappimage";

    if (!prefix.empty()) {
        tmpl += "-";
        tmpl += prefix;
    }

    tmpl += "XXXXXX";

    std::vector<char> tmplC(tmpl.string().size() + 1);
    strncpy(tmplC.data(), tmpl.c_str(), tmpl.string().size());

    if (mkdtemp(tmplC.data()) == nullptr) {
        std::stringstream message;
        message << "failed to create temporary directory using template " << tmpl << ": " << strerror(errno);

        throw std::runtime_error(message.str());
    }

    _path = std::string(tmplC.data());

    if (!std::filesystem::is_directory(_path)) {
        throw std::runtime_error("temporary directory " + _path.string() + " does not exist");
    }
}

TemporaryDirectory::~TemporaryDirectory() {
    std::filesystem::remove_all(_path);
}

std::filesystem::path TemporaryDirectory::path() const {
    return _path;
}
