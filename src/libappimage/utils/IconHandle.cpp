// system
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>

// local
#include "DLHandle.h"
#include "IconHandle.h"

#include "IconHandleCairoRsvg.h"

class appimage::utils::IconHandle::Priv : public IconHandleCairoRsvg {
public:
    Priv(const std::vector<char>& data) : IconHandleCairoRsvg(data) {}

    Priv(const std::string& path) : IconHandleCairoRsvg(path) {}
};

namespace appimage {
    namespace utils {

        IconHandle::IconHandle(std::vector<char>& data) : d(new Priv(data)) {}

        int IconHandle::getSize() const { return d->getSize(); }

        std::string IconHandle::format() const { return d->getFormat(); }

        void IconHandle::setSize(int size) { d->setSize(size); }

        void IconHandle::save(const std::string& path, const std::string& format) const {
            std::filesystem::path bPath(path);
            try { std::filesystem::create_directories(bPath.parent_path()); }
            catch (const std::filesystem::filesystem_error&) { throw IconHandleError("Unable to create parent path"); }

            d->save(bPath, format);
        }

        IconHandle::IconHandle(const std::string& path) : d(new Priv(path)) {}

        IconHandle::~IconHandle() = default;

        IconHandleError::IconHandleError(const std::string& what) : runtime_error(what) {}
    }
}

