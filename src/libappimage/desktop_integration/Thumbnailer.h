// system
#include <string>

// local
#include "integrator/DesktopIntegrationResources.h"

namespace appimage {
    namespace desktop_integration {
        /**
         * Thumbnails generator for AppImage files
         *
         * Follows the Thumbnail Managing Standard by FreeDesktop
         * https://specifications.freedesktop.org/thumbnail-spec/0.8.0/index.html
         */
        class Thumbnailer {
            std::string xdgCacheHome;

        public:
            explicit Thumbnailer(const std::string& xdgCacheHome = "");

            /**
             * @brief Generate thumbnails for the given <appImagePath>
             *
             * Thumbnail generation is performed according to the Freedesktop specification.
             * Two images of 128x128 and 256x256 will be placed at "$XDG_CACHE_HOME/thumbnails/normal" and
             * "$XDG_CACHE_HOME/thumbnails/large/" respectively. The thumbnails name will be formed by a md5 sum of
             * the absolute canonical URI for the original file whit ".png" as extension.
             *
             * Full FreeDesktop Thumbnails spec: https://specifications.freedesktop.org/thumbnail-spec/0.8.0/x227.html
             *
             * @param appImagePath
             */
            void create(const std::string& appImagePath);

            virtual ~Thumbnailer();

            void remove(const std::string& appImagePath);

        private:
            boost::filesystem::path getNormalThumbnailPath(const std::string& canonicalPathMd5) const;

            boost::filesystem::path getLargeThumbnailPath(const std::string& canonicalPathMd5) const;

            std::string getCanonicalPathMd5(const std::string& appImagePath) const;

            DesktopIntegrationResources extractResources(const std::string& appImagePath) const;

            std::string getAppIconName(const DesktopIntegrationResources& resources) const;


            std::vector<char> getIconData(const DesktopIntegrationResources& resources,
                                          const std::string& appIcon, const std::string& iconSize);

            void generateNormalSizeThumbnail(const std::string& canonicalPathMd5,
                                             std::vector<char>& normalIconData) const;

            void generateLargeSizeThumbnail(const std::string& canonicalPathMd5,
                                            std::vector<char>& largeIconData) const;
        };
    }
}

