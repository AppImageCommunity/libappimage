// system
#include <sstream>
#include <iostream>

// libraries
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>


// local
#include "DLHandle.h"
#include "IconHandle.h"

namespace bf = boost::filesystem;
namespace io = boost::iostreams;


namespace appimage {
    namespace utils {
        class IconHandle::Priv {

            struct RSvgHandle : protected DLHandle {
                // rsvg API symbols
                struct RSvgDimensionData {
                    int width;
                    int height;
                    double em;
                    double ex;
                };

                typedef void* (* rsvg_handle_new_from_data_t)(const uint8_t* data, unsigned long data_len,
                                                              void** error);

                typedef void* (* rsvg_handle_new_from_file_t )(const char* file_name, void** error);

                typedef bool (* rsvg_handle_render_cairo_t)(void* handle, void* cr);

                typedef void(* rsvg_handle_get_dimensions_t)(void* handle, RSvgDimensionData* dimension_data);

                /**
                 * @brief Load librsvg-2 and resolve the symbol addresses required by the IconHandle.
                 *
                 * Mode comments:
                 * RTLD_LAZY - load the lib only the required symbols
                 * RTLD_NODELETE - do not unload the lib, as it wasn't designed to be used this way it
                 *                  will produce a big crash.
                 */
                RSvgHandle() : DLHandle("librsvg-2.so.2", RTLD_LAZY | RTLD_NODELETE) {
                    DLHandle::loadSymbol(handle_new_from_data, "rsvg_handle_new_from_data");
                    DLHandle::loadSymbol(handle_render_cairo, "rsvg_handle_render_cairo");
                    DLHandle::loadSymbol(handle_get_dimensions, "rsvg_handle_get_dimensions");
                    DLHandle::loadSymbol(handle_new_from_file, "rsvg_handle_new_from_file");
                }

                rsvg_handle_new_from_data_t handle_new_from_data = nullptr;
                rsvg_handle_render_cairo_t handle_render_cairo = nullptr;
                rsvg_handle_get_dimensions_t handle_get_dimensions = nullptr;
                rsvg_handle_new_from_file_t handle_new_from_file = nullptr;
            };

            class CairoHandle : protected DLHandle {
                // cairo API symbols
                typedef enum _cairo_status {
                    CAIRO_STATUS_SUCCESS = 0,

                    CAIRO_STATUS_NO_MEMORY,
                    CAIRO_STATUS_INVALID_RESTORE,
                    CAIRO_STATUS_INVALID_POP_GROUP,
                    CAIRO_STATUS_NO_CURRENT_POINT,
                    CAIRO_STATUS_INVALID_MATRIX,
                    CAIRO_STATUS_INVALID_STATUS,
                    CAIRO_STATUS_NULL_POINTER,
                    CAIRO_STATUS_INVALID_STRING,
                    CAIRO_STATUS_INVALID_PATH_DATA,
                    CAIRO_STATUS_READ_ERROR,
                    CAIRO_STATUS_WRITE_ERROR,
                    CAIRO_STATUS_SURFACE_FINISHED,
                    CAIRO_STATUS_SURFACE_TYPE_MISMATCH,
                    CAIRO_STATUS_PATTERN_TYPE_MISMATCH,
                    CAIRO_STATUS_INVALID_CONTENT,
                    CAIRO_STATUS_INVALID_FORMAT,
                    CAIRO_STATUS_INVALID_VISUAL,
                    CAIRO_STATUS_FILE_NOT_FOUND,
                    CAIRO_STATUS_INVALID_DASH,
                    CAIRO_STATUS_INVALID_DSC_COMMENT,
                    CAIRO_STATUS_INVALID_INDEX,
                    CAIRO_STATUS_CLIP_NOT_REPRESENTABLE,
                    CAIRO_STATUS_TEMP_FILE_ERROR,
                    CAIRO_STATUS_INVALID_STRIDE,
                    CAIRO_STATUS_FONT_TYPE_MISMATCH,
                    CAIRO_STATUS_USER_FONT_IMMUTABLE,
                    CAIRO_STATUS_USER_FONT_ERROR,
                    CAIRO_STATUS_NEGATIVE_COUNT,
                    CAIRO_STATUS_INVALID_CLUSTERS,
                    CAIRO_STATUS_INVALID_SLANT,
                    CAIRO_STATUS_INVALID_WEIGHT,
                    CAIRO_STATUS_INVALID_SIZE,
                    CAIRO_STATUS_USER_FONT_NOT_IMPLEMENTED,
                    CAIRO_STATUS_DEVICE_TYPE_MISMATCH,
                    CAIRO_STATUS_DEVICE_ERROR,
                    CAIRO_STATUS_INVALID_MESH_CONSTRUCTION,
                    CAIRO_STATUS_DEVICE_FINISHED,
                    CAIRO_STATUS_JBIG2_GLOBAL_MISSING,
                    CAIRO_STATUS_PNG_ERROR,
                    CAIRO_STATUS_FREETYPE_ERROR,
                    CAIRO_STATUS_WIN32_GDI_ERROR,
                    CAIRO_STATUS_TAG_ERROR,

                    CAIRO_STATUS_LAST_STATUS
                } cairo_status_t;

                typedef void* (* cairo_image_surface_create_t)(int format, int width, int height);

                typedef void* (* cairo_create_t)(void* target);

                typedef int (* cairo_write_func_t)(void* closure, const unsigned char* data,
                                                   unsigned int length);

                typedef int (* cairo_surface_write_to_png_stream_t)(void* surface, cairo_write_func_t write_func,
                                                                    void* closure);

                typedef void (* cairo_destroy_t)(void* cr);

                typedef void (* cairo_surface_destroy_t)(void* surface);

                typedef void (* cairo_scale_t)(void* cr, double sx, double sy);

                typedef cairo_status_t (* cairo_read_func_t)(void* closure,
                                                             unsigned char* data,
                                                             unsigned int length);

                typedef void* (* cairo_image_surface_create_from_png_stream_t)(cairo_read_func_t read_func,
                                                                               void* closure);

                typedef void* (* cairo_image_surface_create_from_png_t )(const char* filename);

                typedef int (* cairo_surface_status_t)(void* surface);

                typedef int (* cairo_image_surface_get_height_t)(void* surface);

                typedef const char* (* cairo_status_to_string_t)(int status);


            public:
                /**
                 * @brief Load libcairo.so.2 and resolve the symbol addresses required by the IconHandle.
                 *
                 * Mode comments:
                 * RTLD_LAZY - load the lib only the required symbols
                 * RTLD_NODELETE - do not unload the lib, as it wasn't designed to be used this way it
                 *                  will produce a big crash.
                 */
                CairoHandle() : DLHandle("libcairo.so.2", RTLD_LAZY | RTLD_NODELETE) {
                    DLHandle::loadSymbol(image_surface_create, "cairo_image_surface_create");
                    DLHandle::loadSymbol(create, "cairo_create");
                    DLHandle::loadSymbol(surface_write_to_png_stream, "cairo_surface_write_to_png_stream");
                    DLHandle::loadSymbol(destroy, "cairo_destroy");
                    DLHandle::loadSymbol(surface_destroy, "cairo_surface_destroy");
                    DLHandle::loadSymbol(scale, "cairo_scale");
                    DLHandle::loadSymbol(surface_status, "cairo_surface_status");
                    DLHandle::loadSymbol(image_surface_create_from_png_stream,
                                         "cairo_image_surface_create_from_png_stream");
                    DLHandle::loadSymbol(image_surface_create_from_png, "cairo_image_surface_create_from_png");
                    DLHandle::loadSymbol(image_surface_get_height, "cairo_image_surface_get_height");
                    DLHandle::loadSymbol(status_to_string, "cairo_status_to_string");
                }


                static int cairoWriteFunc(void* closure, const unsigned char* data, unsigned int length) {
                    auto outData = static_cast<std::vector<uint8_t>*>(closure);

                    auto offset = static_cast<unsigned int>(outData->size());
                    outData->resize(offset + length);

                    memcpy(outData->data() + offset, data, length);

                    return CAIRO_STATUS_SUCCESS;
                }

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

                cairo_image_surface_create_t image_surface_create = nullptr;
                cairo_create_t create = nullptr;
                cairo_surface_write_to_png_stream_t surface_write_to_png_stream = nullptr;
                cairo_destroy_t destroy = nullptr;
                cairo_surface_destroy_t surface_destroy = nullptr;
                cairo_scale_t scale = nullptr;
                cairo_image_surface_create_from_png_stream_t image_surface_create_from_png_stream = nullptr;
                cairo_image_surface_create_from_png_t image_surface_create_from_png = nullptr;
                cairo_surface_status_t surface_status = nullptr;
                cairo_image_surface_get_height_t image_surface_get_height = nullptr;
                cairo_status_to_string_t status_to_string = nullptr;
            };

            struct GLibOjbectHandle : protected DLHandle {
                // GlibOjbect API symbols
                typedef void (* g_object_unref_t)(void* object);

                g_object_unref_t object_unref = nullptr;

                /**
                 * @brief Load libgobject-2.0.so and resolve the symbol addresses required by the IconHandle.
                 *
                 * Mode comments:
                 * RTLD_LAZY - load the lib only the required symbols
                 * RTLD_NODELETE - do not unload the lib, as it wasn't designed to be used this way it
                 *                  will produce a big crash.
                 */
                GLibOjbectHandle() : DLHandle("libgobject-2.0.so", RTLD_LAZY | RTLD_NODELETE) {
                    DLHandle::loadSymbol(object_unref, "g_object_unref");
                }
            };

            RSvgHandle rsvg;
            CairoHandle cairo;
            GLibOjbectHandle glibOjbect;


            std::vector<char> originalData;

            int iconSize;
            int iconOriginalSize;
            std::string imageFormat;

            void* rsvgHandle = nullptr;
            void* cairoSurface = nullptr;

            bool tryLoadSvg(const std::vector<char>& data) {
                rsvgHandle = rsvg.handle_new_from_data(reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                                                       nullptr);

                if (rsvgHandle) {
                    imageFormat = "svg";
                    return true;
                } else
                    return false;
            }

            bool tryLoadPng(const std::vector<char>& data) {
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

            /**
             * Render the svg as an image of size <iconSize>
             * @return raw image data
             */
            std::vector<char> svg2png() {
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

            /**
             * Resize the original image if required
             * @return raw image data
             */
            std::vector<char> png2png() {
                // no transformation required
                if (iconOriginalSize == iconSize)
                    return originalData;
                else
                    throw IconHandleError("png resizing is not supported");
            }

        public:
            explicit Priv(const std::vector<char>& data) {
                // make sure that the data is placed in a contiguous block
                originalData.resize(data.size());
                std::move(data.begin(), data.end(), originalData.begin());

                // guess the image format by trying to load it
                if (!tryLoadPng(originalData) && !tryLoadSvg(originalData)) {
                    throw IconHandleError("Unable to load image.");
                }

                iconSize = iconOriginalSize = getOriginalSize();
            }

            explicit Priv(const std::string& path) {
                readFile(path);

                // guess the image format by trying to load it
                if (!tryLoadPng(originalData) && !tryLoadSvg(originalData))
                    throw IconHandleError("Unable to load image.");

                iconSize = iconOriginalSize = getOriginalSize();
            }

            void readFile(const std::string& path) {
                std::ifstream in(path, std::ios_base::in | std::ios_base::binary | std::ios_base::ate);

                auto size = static_cast<unsigned long>(in.tellg());
                originalData.resize(size);

                in.seekg(0, std::ios_base::beg);
                in.read(reinterpret_cast<char*>(originalData.data()), size);
            }


            virtual ~Priv() {
                if (cairoSurface != nullptr)
                    cairo.surface_destroy(cairoSurface);

                if (rsvgHandle != nullptr)
                    glibOjbect.object_unref(rsvgHandle);
            }

            int getOriginalSize() {
                // Icons are squared so we only have to query for one size value
                if (imageFormat == "png" && cairoSurface != nullptr)
                    return cairo.image_surface_get_height(cairoSurface);


                if (imageFormat == "svg" && rsvgHandle != nullptr) {
                    Priv::RSvgHandle::RSvgDimensionData dimensions = {};
                    rsvg.handle_get_dimensions(rsvgHandle, &dimensions);

                    return dimensions.height;
                }

                throw IconHandleError("Malformed IconHandle");
            }

            int getIconSize() const { return iconSize; }

            void setIconSize(int iconSize) { Priv::iconSize = iconSize; }

            const std::string& getImageFormat() const { return imageFormat; }

            void save(const boost::filesystem::path& path, const std::string& targetFormat) {
                const auto& output = getNewIconData(targetFormat);

                if (output.empty())
                    throw IconHandleError("Unable to transform " + imageFormat + " into " + targetFormat);

                bf::ofstream ofstream(path, std::ios::out | std::ios::binary | std::ios::trunc);
                if (ofstream.is_open())
                    ofstream.write(reinterpret_cast<const char*>(output.data()), output.size());
                else
                    throw IconHandleError("Unable to write into: " + path.string());
            }

            std::vector<char> getNewIconData(const std::string& targetFormat) {
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
            }
        };

        IconHandle::IconHandle(std::vector<char>& data) : priv(new Priv(data)) {}

        int IconHandle::getSize() { return priv->getIconSize(); }

        std::string IconHandle::format() { return priv->getImageFormat(); }

        void IconHandle::setSize(int size) { priv->setIconSize(size); }

        void IconHandle::save(const std::string& path, const std::string& format) {
            bf::path bPath(path);
            try { bf::create_directories(bPath.parent_path()); }
            catch (const bf::filesystem_error&) { throw IconHandleError("Unable to create parent path"); }

            priv->save(bPath, format);
        }

        IconHandle::IconHandle(const std::string& path) : priv(new Priv(path)) {}

        IconHandle::~IconHandle() = default;

        IconHandleError::IconHandleError(const std::string& what) : runtime_error(what) {}

    }
}

