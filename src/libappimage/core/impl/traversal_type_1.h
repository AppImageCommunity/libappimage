#pragma once
// system
#include <memory>

// local
#include "core/traversal.h"

namespace appimage {
    namespace core {
        namespace impl {
            /**
             * Provides an implementation of the traversal class for type 1 AppImages. It's based on libarchive.
             * As libarchive imposes this is a READONLY, ONE WAY, SINGLE PASS traversal implementation.
             *
             * See the base class for more details.
             */
            class traversal_type_1 : public traversal {
                std::string path;
                struct archive* a = {nullptr};
                struct archive_entry* entry = {nullptr};

                std::shared_ptr<std::istream> appImageIStream;
                bool completed = false;
            public:
                explicit traversal_type_1(const std::string& path);

                ~traversal_type_1() override;

                void next() override;

                bool isCompleted() override;

                std::string getEntryName() override;

                void extract(const std::string& target) override;

                std::istream& read() override;
            };
        }
    }
}

