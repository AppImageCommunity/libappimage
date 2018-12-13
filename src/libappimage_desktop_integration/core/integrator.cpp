#include "integrator.h"

using namespace appimage::desktop_integration::core;

/**
 * Implementation of the opaque pointer patter for the integrator class
 * see https://en.wikipedia.org/wiki/Opaque_pointer
 */
struct integrator::priv {
    std::string path;
    std::string xdg_data_dir;
};

integrator::integrator(const std::string& path) : d_ptr(new priv) {
    d_ptr->path = path;
}

integrator::integrator(std::string path, std::string xdg_data_dir) : d_ptr(new priv) {
    d_ptr->path = path;
    d_ptr->xdg_data_dir = xdg_data_dir;

}

integrator::~integrator() = default;

void integrator::integrate() {

}
