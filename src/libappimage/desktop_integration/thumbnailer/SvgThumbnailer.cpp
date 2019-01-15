// system
#include <dlfcn.h>
#include <ostream>
#include <sstream>

// local
#include "SvgThumbnailer.h"
#include "Exceptions.h"
extern "C" {
#include <glib-2.0/glib-object.h>
}

namespace appimage {
    namespace desktop_integration {
        namespace thumbnailer {

            class SvgThumbnailer::Priv {

                struct DLHandle {
                    std::string libName;
                    void* handle;

                    explicit DLHandle(const std::string& libName) : libName(libName) {
                        handle = dlopen(libName.c_str(), RTLD_LAZY | RTLD_NODELETE);

                        if (!handle)
                            throw ThumbnailerError("Unable to load " + libName);
                    }

                    virtual ~DLHandle() {
                        dlclose(handle);
                    }

                    template<typename T>
                    void loadSymbol(T& symbol, const std::string& symbolName) {
                        symbol = (T) dlsym(handle, symbolName.c_str());

                        if (symbol == nullptr)
                            throw RSvgError("Unable to load " + libName + " symbol: " + symbolName);
                    }
                };

                struct RSvgApi : DLHandle {
                    // rsvg API symbols
                    struct RsvgDimensionData {
                        int width;
                        int height;
                        gdouble em;
                        gdouble ex;
                    };

                    typedef void* (* rsvg_handle_new_from_data_t)(const char* data, unsigned long data_len,
                                                                  void** error);

                    typedef bool (* rsvg_handle_render_cairo_t)(void* handle, void* cr);

                    typedef void(* rsvg_handle_get_dimensions_t)(void* handle, RsvgDimensionData* dimension_data);

                    RSvgApi() : DLHandle("librsvg-2.so.2") {
                        loadSymbol(handle_new_from_data, "rsvg_handle_new_from_data");
                        loadSymbol(handle_render_cairo, "rsvg_handle_render_cairo");
                        loadSymbol(handle_get_dimensions, "rsvg_handle_get_dimensions");
                    }

                    rsvg_handle_new_from_data_t handle_new_from_data = nullptr;
                    rsvg_handle_render_cairo_t handle_render_cairo = nullptr;
                    rsvg_handle_get_dimensions_t handle_get_dimensions = nullptr;
                };

                struct CairoApi : DLHandle {
                    // cairo API symbols
                    typedef void* (* cairo_image_surface_create_t)(int format, int width, int height);

                    typedef void* (* cairo_create_t)(void* target);

                    typedef int(* cairo_write_func_t)(void* closure, const unsigned char* data,
                                                      unsigned int length);

                    typedef int (* cairo_surface_write_to_png_stream_t)(void* surface, cairo_write_func_t write_func,
                                                                        void* closure);

                    typedef void (* cairo_destroy_t)(void* cr);

                    typedef void (* cairo_surface_destroy_t)(void* surface);

                    typedef void (* cairo_scale_t)(void* cr, double sx, double sy);


                    static int cairoWriteFunc(void* closure, const unsigned char* data, unsigned int length) {
                        auto output = static_cast<std::vector<char>*>(closure);
                        output->insert(output->end(), data, data + length);

                        return 0;
                    }

                    CairoApi() : DLHandle("libcairo.so.2") {
                        loadSymbol(image_surface_create, "cairo_image_surface_create");
                        loadSymbol(create, "cairo_create");
                        loadSymbol(surface_write_to_png_stream, "cairo_surface_write_to_png_stream");
                        loadSymbol(destroy, "cairo_destroy");
                        loadSymbol(surface_destroy, "cairo_surface_destroy");
                        loadSymbol(scale, "cairo_scale");
                    }

                    cairo_image_surface_create_t image_surface_create = nullptr;
                    cairo_create_t create = nullptr;
                    cairo_surface_write_to_png_stream_t surface_write_to_png_stream = nullptr;
                    cairo_destroy_t destroy = nullptr;
                    cairo_surface_destroy_t surface_destroy = nullptr;
                    cairo_scale_t scale = nullptr;
                };

            public:
                std::vector<char> createThumbnail(const std::vector<char>& data, unsigned int size) {
                    // load svg
                    void* handle = rsvg.handle_new_from_data(data.data(), data.size(), nullptr);

                    if (!handle)
                        throw ThumbnailerError("Unable create an image handler.");

                    // Scale Image
                    RSvgApi::RsvgDimensionData dimensions = {};
                    double xFactor, yFactor;
                    double scale_factor;

                    rsvg.handle_get_dimensions(handle, &dimensions);

                    xFactor = (double) size / dimensions.width;
                    yFactor = (double) size / dimensions.height;

                    scale_factor = std::min(xFactor, yFactor);

                    // prepare cairo rendering surface
                    void* surface = cairo.image_surface_create(0, size, size);
                    void* cr = cairo.create(surface);

                    // set scale factor
                    cairo.scale(cr, scale_factor, scale_factor);

                    // render
                    rsvg.handle_render_cairo(handle, cr);


                    // write stream
                    std::vector<char> out;
                    cairo.surface_write_to_png_stream(surface, CairoApi::cairoWriteFunc, &out);

                    // clean
                    cairo.destroy(cr);
                    cairo.surface_destroy(surface);

                    g_object_unref(handle);

                    return out;
                }

                RSvgApi rsvg;
                CairoApi cairo;
            };

            SvgThumbnailer::SvgThumbnailer() : priv(new Priv) {}

            std::vector<char> SvgThumbnailer::create(const std::vector<char>& vector, unsigned int size) {
                return priv->createThumbnail(vector, size);
            }

            SvgThumbnailer::~SvgThumbnailer() = default;
        }
    }
}
