// system
#include <string>

// libraries
#include <filesystem>

// local
#include <appimage/core/AppImage.h>
#include <appimage/utils/ResourcesExtractor.h>

namespace appimage {
    namespace desktop_integration {
        /**
         * Thumbnails generator for AppImage files
         *
         * Follows the Thumbnail Managing Standard by FreeDesktop
         * https://specifications.freedesktop.org/thumbnail-spec/0.8.0/index.html
         */
        class Thumbnailer {
        public:
            /**
             * Creates a Thumbnailer that will create and remove thumbnails at the user XDG_CACHE_HOME dir.
             */
            explicit Thumbnailer();

            /**
             * Creates a Thumbnailer that will create and remove thumbnails at the dir pointed by <xdgCacheHome> .
             */
            explicit Thumbnailer(const std::string& xdgCacheHome);

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
             * @param appImage
             */
            void create(const core::AppImage& appImage) const;

            /**
             * @brief remove <appImage> thumbnails
             *
             * Will find and remove every thumbnail related to the file pointed by the AppImage path. The files will
             * be identified following the rules described in the Full FreeDesktop Thumbnails spec. Which is available
             * at: https://specifications.freedesktop.org/thumbnail-spec/0.8.0/x227.html
             * @param appImagePath
             */
            void remove(const std::string& appImagePath) const;

            virtual ~Thumbnailer();

        private:
            std::filesystem::path xdgCacheHome;

            static constexpr const char* thumbnailFileExtension = ".png";

            static constexpr const char* normalThumbnailsPrefix = "thumbnails/normal";

            std::filesystem::path getNormalThumbnailPath(const std::string& canonicalPathMd5) const;

            static constexpr const char* largeThumbnailPrefix = "thumbnails/large";

            std::filesystem::path getLargeThumbnailPath(const std::string& canonicalPathMd5) const;

            std::string getAppIconName(const utils::ResourcesExtractor& resourcesExtractor) const;

            std::string getIconPath(std::vector<std::string> appIcons, const std::string& size) const;

            void generateNormalSizeThumbnail(const std::string& canonicalPathMd5,
                                             std::vector<char>& normalIconData) const;

            void generateLargeSizeThumbnail(const std::string& canonicalPathMd5,
                                            std::vector<char>& largeIconData) const;
        };
    }
}


