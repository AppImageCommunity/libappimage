// system
#include <memory>

// local
#include "core/Traversal.h"

namespace appimage {
    namespace core {
        namespace impl {
            /**
             * Provides a fallback implementation of the traversal class to be used when an unknown AppImage format
             * is used, an error occur or a dummy traversal implementation is required (by example at the end of an
             * files_iterator).
             *
             * See the base class for more details.
             */
            class traversal_fallback : public Traversal {
                std::shared_ptr<std::istream> fileStream;
            public:
                void next() override;

                bool isCompleted() override;

                std::string getEntryName() override;

                void extract(const std::string& target) override;

                std::istream& read() override;
            };
        }
    }
}
