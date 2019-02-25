// local
#include "PayloadEntriesCache.h"

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            PayloadEntriesCache::PayloadEntriesCache(const core::AppImage& image) : appImage(image) {
                buildCache();
            }

            std::vector<std::string> PayloadEntriesCache::getEntriesPaths() const {
                std::vector<std::string> paths;
                for (const auto& item: entriesCache)
                    paths.emplace_back(item.first);

                return paths;
            }

            appimage::core::PayloadEntryType PayloadEntriesCache::getEntryType(const std::string& path) const {
                auto itr = entriesCache.find(path);
                if (itr == entriesCache.end())
                    throw core::PayloadIteratorError("Entry doesn't exists: " + path);

                return itr->second;
            }

            std::string PayloadEntriesCache::getEntryLinkTarget(const std::string& path) const {
                auto itr = linksCache.find(path);
                if (itr == linksCache.end())
                    throw core::PayloadIteratorError("Not a link: " + path);

                if (itr->second.empty())
                    throw core::PayloadIteratorError("Loop found: " + path);
                else
                    return itr->second;
            }

            void PayloadEntriesCache::buildCache() {
                readAllEntries();
                resolveLinks();
            }

            void PayloadEntriesCache::resolveLinks() {
                for (auto itr = linksCache.begin(); itr != linksCache.end(); ++itr) {
                    auto target = itr->second;
                    auto jumpItr = linksCache.find(itr->second);

                    // follow links to the final target
                    while (jumpItr != linksCache.end() && jumpItr != itr) {
                        target = jumpItr->second;
                        jumpItr = linksCache.find(jumpItr->second);
                    }

                    // disable loops
                    if (target == itr->first)
                        target = "";

                    // update cache
                    itr->second = target;
                }
            }

            void PayloadEntriesCache::readAllEntries() {
                for (auto fileItr = appImage.files(); fileItr != fileItr.end(); ++fileItr) {
                    entriesCache[fileItr.path()] = fileItr.type();

                    if (fileItr.type() == core::PayloadEntryType::LINK)
                        linksCache[fileItr.path()] = fileItr.linkTarget();
                }
            }
        }
    }
}
