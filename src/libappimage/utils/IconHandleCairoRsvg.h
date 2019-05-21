#pragma once
// libraries
extern "C" {
#include <cairo-svg.h>
#include <librsvg/rsvg.h>
}

// local
#include "IconHandlePriv.h"

namespace appimage {
    namespace utils {

        class IconHandleCairoRsvg : public IconHandlePriv {
        public:
            explicit IconHandleCairoRsvg(const std::vector<char>& data);

            explicit IconHandleCairoRsvg(const std::string& path);

            ~IconHandleCairoRsvg() override;

            int getOriginalSize() override;

            int getSize() const override;

            void setSize(int newSize) override;

            const std::string& getFormat() const override;

            void save(const boost::filesystem::path& path, const std::string& targetFormat) override;

        private:
            std::vector<char> originalData;

            int iconSize;
            int iconOriginalSize;
            std::string imageFormat;

            RsvgHandle* rsvgHandle = nullptr;
            cairo_surface_t* cairoSurface = nullptr;

            bool tryLoadSvg(const std::vector<char>& data);

            bool tryLoadPng(const std::vector<char>& data);

            /**
             * Render the svg as an image of size <iconSize>
             * @return raw image data
             */
            std::vector<char> svg2png();

            /**
             * Resize the original image if required
             * @return raw image data
             */
            std::vector<char> png2png();

            void readFile(const std::string& path);

            std::vector<char> getNewIconData(const std::string& targetFormat);
        };
    }
}
