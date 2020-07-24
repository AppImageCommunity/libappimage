#include "IconHandleDLOpenCairoRsvg.h"

namespace appimage {
    namespace utils {
        IconHandleDLOpenCairoRsvg::IconHandleDLOpenCairoRsvg(const std::vector<char>& data) : IconHandlePriv(data) {
            // make sure that the data is placed in a contiguous block
            originalData.resize(data.size());
            std::move(data.begin(), data.end(), originalData.begin());

            // guess the image format by trying to load it
            if (!tryLoadPng(originalData) && !tryLoadSvg(originalData)) {
                throw IconHandleError("Unable to load image.");
            }

            iconSize = iconOriginalSize = getOriginalSize();
        }

        IconHandleDLOpenCairoRsvg::IconHandleDLOpenCairoRsvg(const std::string& path) : IconHandlePriv(path) {
            readFile(path);

            // guess the image format by trying to load it
            if (!tryLoadPng(originalData) && !tryLoadSvg(originalData))
                throw IconHandleError("Unable to load image.");

            iconSize = iconOriginalSize = getOriginalSize();
        }

        IconHandleDLOpenCairoRsvg::~IconHandleDLOpenCairoRsvg() {
            if (cairoSurface != nullptr)
                cairo.surface_destroy(cairoSurface);

            if (rsvgHandle != nullptr)
                glibOjbect.object_unref(rsvgHandle);
        }

        int IconHandleDLOpenCairoRsvg::getOriginalSize() {
            // Icons are squared so we only have to query for one size value
            if (imageFormat == "png" && cairoSurface != nullptr)
                return cairo.image_surface_get_height(cairoSurface);


            if (imageFormat == "svg" && rsvgHandle != nullptr) {
                IconHandleDLOpenCairoRsvg::RSvgHandle::RSvgDimensionData dimensions = {};
                rsvg.handle_get_dimensions(rsvgHandle, &dimensions);

                return dimensions.height;
            }

            throw IconHandleError("Malformed IconHandle");
        }

        int IconHandleDLOpenCairoRsvg::getSize() const { return iconSize; }

        void IconHandleDLOpenCairoRsvg::setSize(int newSize) { IconHandleDLOpenCairoRsvg::iconSize = newSize; }

        const std::string& IconHandleDLOpenCairoRsvg::getFormat() const { return imageFormat; }

        void IconHandleDLOpenCairoRsvg::save(const boost::filesystem::path& path, const std::string& targetFormat) {
            const auto& output = getNewIconData(targetFormat);

            if (output.empty())
                throw IconHandleError("Unable to transform " + imageFormat + " into " + targetFormat);

            std::ofstream ofstream(path.string(), std::ios::out | std::ios::binary | std::ios::trunc);
            if (ofstream.is_open())
                ofstream.write(reinterpret_cast<const char*>(output.data()), output.size());
            else
                throw IconHandleError("Unable to write into: " + path.string());
        }

        bool IconHandleDLOpenCairoRsvg::tryLoadSvg(const std::vector<char>& data) {
            rsvgHandle = rsvg.handle_new_from_data(reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                                                   nullptr);

            if (rsvgHandle) {
                imageFormat = "svg";
                return true;
            } else
                return false;
        }

        bool IconHandleDLOpenCairoRsvg::tryLoadPng(const std::vector<char>& data) {
            CairoHandle::ReadCtx readCtx(reinterpret_cast<const uint8_t*>(data.data()), data.size());
            cairoSurface = cairo.image_surface_create_from_png_stream(CairoHandle::cairoReadFunc, &readCtx);

            auto status = cairo.surface_status(cairoSurface);
            if (status == 0) {
                // All went ok, we have a PNG image
                imageFormat = "png";
                return true;
            } else {
                // It's not a PNG, let's clean up that surface
                cairo.surface_destroy(cairoSurface);
                cairoSurface = nullptr;
                return false;
            }
        }

        std::vector<char> IconHandleDLOpenCairoRsvg::svg2png() {
            // prepare cairo rendering surface
            void* surface = cairo.image_surface_create(0, iconSize, iconSize);
            void* cr = cairo.create(surface);


            if (iconOriginalSize != iconSize && iconOriginalSize != 0) {
                // Scale Image
                double scale_factor = iconSize / iconOriginalSize;

                // set scale factor
                cairo.scale(cr, scale_factor, scale_factor);
            }

            // render
            rsvg.handle_render_cairo(rsvgHandle, cr);

            std::vector<char> out;
            cairo.surface_write_to_png_stream(surface, CairoHandle::cairoWriteFunc, &out);

            // clean
            cairo.destroy(cr);
            cairo.surface_destroy(surface);

            return out;
        }

        std::vector<char> IconHandleDLOpenCairoRsvg::png2png() {
            // no transformation required
            if (iconOriginalSize == iconSize)
                return originalData;
            else
                throw IconHandleError("png resizing is not supported");
        }

        void IconHandleDLOpenCairoRsvg::readFile(const std::string& path) {
            std::ifstream in(path, std::ios_base::in | std::ios_base::binary | std::ios_base::ate);

            auto size = static_cast<unsigned long>(in.tellg());
            originalData.resize(size);

            in.seekg(0, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(originalData.data()), size);
        }

        std::vector<char> IconHandleDLOpenCairoRsvg::getNewIconData(const std::string& targetFormat) {
            if (targetFormat == "png") {
                if (imageFormat == "svg")
                    return svg2png();

                if (imageFormat == "png")
                    return png2png();
            }

            if (targetFormat == "svg") {
                if (imageFormat == "svg")
                    return originalData; // svgs doens't require to be resized

                if (imageFormat == "png")
                    throw IconHandleError("png to svg conversion is not supported");
            }

            throw IconHandleError("Unsuported format");
        }
    }
}
