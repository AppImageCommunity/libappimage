#pragma once
// system
#include <memory>

// local
#include "core/Traversal.h"

namespace appimage {
    namespace core {
        namespace impl {
            /**
             * Provides an implementation of the traversal class for type 1 AppImages. It's based on libarchive.
             * As libarchive imposes this is a READONLY, ONE WAY, SINGLE PASS traversal implementation.
             *
             * See the base class for more details.
             */
            class TraversalType1 : public Traversal {
                std::string path;
                struct archive* a = {nullptr};
                struct archive_entry* entry = {nullptr};

                std::shared_ptr<std::istream> appImageIStream;
                bool completed = false;
            public:
                explicit TraversalType1(const std::string& path);

                ~TraversalType1() override;

                void next() override;

                bool isCompleted() override;

                std::string getEntryName() override;

                entry::Type getEntryType() override;

                void extract(const std::string& target) override;

                std::istream& read() override;
            };
        }
    }
}

