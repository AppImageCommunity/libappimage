#pragma once

// system
#include <vector>
#include <memory>

namespace appimage {
    namespace desktop_integration {

        namespace thumbnailer {
            /**
             * Allows to generate Thumbnails for SVG Images
             */
            class SvgThumbnailer {
            public:
                explicit SvgThumbnailer();

                std::vector<char> create(const std::vector<char>& vector, unsigned int size);

                virtual ~SvgThumbnailer();

            private:
                class Priv;
                std::unique_ptr<Priv> priv;
            };
        }
    }
}
