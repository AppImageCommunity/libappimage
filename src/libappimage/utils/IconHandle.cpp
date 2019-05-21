// system
#include <sstream>
#include <iostream>
#include <fstream>

// libraries
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>


// local
#include "DLHandle.h"
#include "IconHandle.h"

namespace bf = boost::filesystem;
namespace io = boost::iostreams;


#include "IconHandleDLOpenCairoRsvg.h"

class appimage::utils::IconHandle::Priv : public IconHandleDLOpenCairoRsvg {
public:
    Priv(const std::vector<char>& data) : IconHandleDLOpenCairoRsvg(data) {}

    Priv(const std::string& path) : IconHandleDLOpenCairoRsvg(path) {}
};

namespace appimage {
    namespace utils {

        IconHandle::IconHandle(std::vector<char>& data) : d(new Priv(data)) {}

        int IconHandle::getSize() { return d->getSize(); }

        std::string IconHandle::format() { return d->getFormat(); }

        void IconHandle::setSize(int size) { d->setSize(size); }

        void IconHandle::save(const std::string& path, const std::string& format) {
            bf::path bPath(path);
            try { bf::create_directories(bPath.parent_path()); }
            catch (const bf::filesystem_error&) { throw IconHandleError("Unable to create parent path"); }

            d->save(bPath, format);
        }

        IconHandle::IconHandle(const std::string& path) : d(new Priv(path)) {}

        IconHandle::~IconHandle() = default;

        IconHandleError::IconHandleError(const std::string& what) : runtime_error(what) {}
    }
}

