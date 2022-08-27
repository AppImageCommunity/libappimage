// libraries
#include <glib-object.h>
#include <fstream>

// local
#include "IconHandle.h"
#include "IconHandleCairoRsvg.h"

namespace appimage {
    namespace utils {

        struct ReadCtx {
            ReadCtx(const uint8_t* data, unsigned int left) : data(data), left(left) {}

            const uint8_t* data;
            unsigned left;
        };

        static _cairo_status cairoReadFunc(void* closure, unsigned char* data, unsigned int length) {
            auto readCtx = static_cast<struct ReadCtx*>(closure);

            if (!readCtx->left)
                return CAIRO_STATUS_READ_ERROR;

            if (length > readCtx->left)
                length = readCtx->left;

            memcpy(data, readCtx->data, length);
            readCtx->data += length;
            readCtx->left -= length;

            return CAIRO_STATUS_SUCCESS;
        }

        static _cairo_status cairoWriteFunc(void* closure,
                                            const unsigned char* data,
                                            unsigned int length) {
            // cast back the vector passed as user parameter on cairo_surface_write_to_png_stream
            // see the cairo_surface_write_to_png_stream doc for details
            auto outData = static_cast<std::vector<uint8_t>*>(closure);

            auto offset = static_cast<unsigned int>(outData->size());
            outData->resize(offset + length);

            memcpy(outData->data() + offset, data, length);

            return CAIRO_STATUS_SUCCESS;
        }

        IconHandleCairoRsvg::IconHandleCairoRsvg(const std::vector<char>& data) : IconHandlePriv(data) {
            // make sure that the data is placed in a contiguous block
            originalData.resize(data.size());
            std::move(data.begin(), data.end(), originalData.begin());

            // guess the image format by trying to load it
            if (!tryLoadPng(originalData) && !tryLoadSvg(originalData)) {
                throw IconHandleError("Unable to load image.");
            }

            iconSize = iconOriginalSize = getOriginalSize();
        }

        IconHandleCairoRsvg::IconHandleCairoRsvg(const std::string& path) : IconHandlePriv(path) {
            readFile(path);

            // guess the image format by trying to load it
            if (!tryLoadPng(originalData) && !tryLoadSvg(originalData))
                throw IconHandleError("Unable to load image.");

            iconSize = iconOriginalSize = getOriginalSize();
        }

        IconHandleCairoRsvg::~IconHandleCairoRsvg() {
            if (cairoSurface != nullptr)
                cairo_surface_destroy(cairoSurface);

            if (rsvgHandle != nullptr)
                g_object_unref(rsvgHandle);
        }

        int IconHandleCairoRsvg::getOriginalSize() {
            // Icons are squared so we only have to query for one size value
            if (imageFormat == "png" && cairoSurface != nullptr)
                return cairo_image_surface_get_height(cairoSurface);


            if (imageFormat == "svg" && rsvgHandle != nullptr) {
                RsvgDimensionData dimensions = {};
                rsvg_handle_get_dimensions(rsvgHandle, &dimensions);

                return dimensions.height;
            }

            throw IconHandleError("Malformed IconHandle");
        }

        int IconHandleCairoRsvg::getSize() const { return iconSize; }

        void IconHandleCairoRsvg::setSize(int newSize) { IconHandleCairoRsvg::iconSize = newSize; }

        const std::string& IconHandleCairoRsvg::getFormat() const { return imageFormat; }

        void IconHandleCairoRsvg::save(const std::filesystem::path& path, const std::string& targetFormat) {
            const auto& output = getNewIconData(targetFormat);

            if (output.empty())
                throw IconHandleError("Unable to transform " + imageFormat + " into " + targetFormat);

            std::ofstream ofstream(path.string(), std::ios::out | std::ios::binary | std::ios::trunc);
            if (ofstream.is_open())
                ofstream.write(reinterpret_cast<const char*>(output.data()), output.size());
            else
                throw IconHandleError("Unable to write into: " + path.string());
        }

        bool IconHandleCairoRsvg::tryLoadSvg(const std::vector<char>& data) {
            rsvgHandle = rsvg_handle_new_from_data(reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                                                   nullptr);

            if (rsvgHandle) {
                imageFormat = "svg";
                return true;
            } else
                return false;
        }

        bool IconHandleCairoRsvg::tryLoadPng(const std::vector<char>& data) {
            ReadCtx readCtx(reinterpret_cast<const uint8_t*>(data.data()), data.size());
            cairoSurface = cairo_image_surface_create_from_png_stream(cairoReadFunc, &readCtx);

            auto status = cairo_surface_status(cairoSurface);
            if (status == 0) {
                // All went ok, we have a PNG image
                imageFormat = "png";
                return true;
            } else {
                // It's not a PNG, let's clean up that surface
                cairo_surface_destroy(cairoSurface);
                cairoSurface = nullptr;
                return false;
            }
        }

        std::vector<char> IconHandleCairoRsvg::svg2png() {
            // prepare cairo rendering surface
            cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, iconSize, iconSize);
            cairo_t* cr = cairo_create(surface);


            if (iconOriginalSize != iconSize && iconOriginalSize != 0) {
                // Scale Image
                double scale_factor = iconSize / iconOriginalSize;

                // set scale factor
                cairo_scale(cr, scale_factor, scale_factor);
            }

            // render
            rsvg_handle_render_cairo(rsvgHandle, cr);

            std::vector<char> out;
            cairo_surface_write_to_png_stream(surface, cairoWriteFunc, &out);

            // clean
            cairo_destroy(cr);
            cairo_surface_destroy(surface);

            return out;
        }

        std::vector<char> IconHandleCairoRsvg::png2png() {
            // no transformation required
            if (iconOriginalSize == iconSize)
                return originalData;
            else {
                // load original image
                ReadCtx readCtx(reinterpret_cast<const uint8_t*>(originalData.data()), originalData.size());
                cairo_surface_t* sourceSurface = cairo_image_surface_create_from_png_stream(cairoReadFunc, &readCtx);

                // prepare cairo rendering surface
                cairo_surface_t* targetSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, iconSize, iconSize);
                cairo_t* cr = cairo_create(targetSurface);

                if (iconOriginalSize != iconSize && iconOriginalSize != 0) {
                    // Scale Image
                    double scale_factor = iconSize / iconOriginalSize;

                    cairo_scale(cr, scale_factor, scale_factor);
                    cairo_set_source_surface(cr, sourceSurface, 0, 0);
                    cairo_paint(cr);
                }


                std::vector<char> out;
                cairo_surface_write_to_png_stream(targetSurface, cairoWriteFunc, &out);

                // clean
                cairo_destroy(cr);
                cairo_surface_destroy(sourceSurface);
                cairo_surface_destroy(targetSurface);

                return out;
            }
        }

        void IconHandleCairoRsvg::readFile(const std::string& path) {
            std::ifstream in(path, std::ios_base::in | std::ios_base::binary | std::ios_base::ate);

            auto size = static_cast<unsigned long>(in.tellg());
            originalData.resize(size);

            in.seekg(0, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(originalData.data()), size);
        }

        std::vector<char> IconHandleCairoRsvg::getNewIconData(const std::string& targetFormat) {
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
