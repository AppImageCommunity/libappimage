#pragma once
// system
#include <memory>

// local
#include "core/Traversal.h"
#include "PayloadIStream.h"
#include "StreambufType1.h"

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

                PayloadIStream entryIStream;
                std::unique_ptr<StreambufType1> entryStreambuf;

                bool completed = false;
            public:
                explicit TraversalType1(const std::string& path);

                // Creating copies of this object is not allowed
                TraversalType1(TraversalType1& other) = delete;

                // Creating copies of this object is not allowed
                TraversalType1& operator=(TraversalType1& other) = delete;

                ~TraversalType1() override;

                void next() override;

                bool isCompleted() const override;

                std::string getEntryName() const override;

                std::string getEntryLink() const override;

                entry::Type getEntryType() const override;

                void extract(const std::string& target) override;

                std::istream& read() override;
            };
        }
    }
}

