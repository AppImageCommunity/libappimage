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
            public:
                explicit TraversalType1(const std::string& path);

                // Creating copies of this object is not allowed
                TraversalType1(TraversalType1& other) = delete;

                // Creating copies of this object is not allowed
                TraversalType1& operator=(TraversalType1& other) = delete;

                ~TraversalType1();

                void next() override;

                bool isCompleted() const override;

                std::string getEntryPath() const override;

                std::string getEntryLinkTarget() const override;

                PayloadEntryType getEntryType() const override;

                void extract(const std::string& target) override;

                std::istream& read() override;

            private:
                // control
                std::string path;
                bool completed = false;

                // libarchive
                struct archive* a = {nullptr};
                struct archive_entry* entry = {nullptr};

                // cache
                std::string entryName;
                PayloadEntryType entryType = PayloadEntryType::UNKNOWN;
                std::string entryLink;
                PayloadIStream entryIStream;
                std::unique_ptr<StreambufType1> entryStreambuf;

                /**
                 * Move to the next header
                 */
                void readNextHeader();

                /**
                 * Read entry data into the cache
                 */
                void readEntryData();

                /**
                 * Read entry name and remove any "." in the prefix
                 * @return current entry name
                 */
                std::string readEntryName();

                /**
                 * Read and map from archive file types to PayloadEntryType.
                 * Hard and Symbolic links are classified as "Links"
                 * @return current entry type
                 */
                PayloadEntryType readEntryType();

                /**
                 * @return entry link if it's a Link type entry otherwise an empty string.
                 */
                std::string readEntryLink();
            };
        }
    }
}

