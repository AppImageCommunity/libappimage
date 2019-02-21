#pragma once

// system
#include <memory>

// local
#include "core/Traversal.h"
#include "PayloadIStream.h"

namespace appimage {
    namespace core {
        namespace impl {
            /**
             * Provides an implementation of the traversal class for type 2 AppImages. It's based on squashfuse.
             * The current implementation has the following limitations: READONLY, ONE WAY, SINGLE PASS.
             *
             * See the base class for more details.
             */
            class TraversalType2 : public Traversal {
            public:
                explicit TraversalType2(std::string path);

                // Creating copies of this object is not allowed
                TraversalType2(TraversalType2& other) = delete;

                // Creating copies of this object is not allowed
                TraversalType2& operator=(TraversalType2& other) = delete;

                ~TraversalType2();

                void next() override;

                bool isCompleted() const override;

                std::string getEntryPath() const override;

                std::string getEntryLinkTarget() const override;

                PayloadEntryType getEntryType() const override;

                void extract(const std::string& target) override;

                std::istream& read() override;

            private:
                // Keep squashfuse private, it's too unstable to go into the wild
                class Priv;

                std::unique_ptr<Priv> d;
            };
        }
    }
}
