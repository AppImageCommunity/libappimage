#pragma once
// system
#include <map>
#include <string>

// local
#include <appimage/core/AppImage.h>
#include <appimage/core/exceptions.h>


namespace appimage {
    namespace utils {
        /**
         * Builds a cache of the entries contained in the AppImage payload. Include the entries path,
         * type and in case of a link link entry the link target. Solve links chains in order to ease
         * the lookup.
         */
        class PayloadEntriesCache {
        public:
            explicit PayloadEntriesCache(const core::AppImage& appImage);

            /**
             * @return entries path inside the AppImage Payload
             */
            std::vector<std::string> getEntriesPaths() const;

            /**
             * Get the type of the entry pointed by <path>
             * @param path
             * @return  entry type
             * @throw PayloadIteratorError if <path> don't point to a existent entry
             */
            appimage::core::PayloadEntryType getEntryType(const std::string& path) const;

            /**
             * Get the final target if a given link entry.
             *
             * Final means that it will point to a regular entry.
             *
             * @param path
             * @return final link target
             * @throw PayloadIteratorError in case links of cycle
             */
            std::string getEntryLinkTarget(const std::string& path) const;

        private:
            core::AppImage appImage;
            std::map<std::string, std::string> linksCache;
            std::map<std::string, appimage::core::PayloadEntryType> entriesCache;

            /**
             * Iterate over all the entries in the AppImage and store all the link type entries
             * and their targets inside linksCache.
             */
            void buildCache();

            /**
             * Fill linksCache with the link file paths and their target
             */
            void readAllEntries();

            /**
             * Resolve links chains to ease the link target lookup.
             *
             * Example scenario:
             * A links to B links to C will be translated to: A links to C
             * */
            void resolveLinks();
        };
    }
}

