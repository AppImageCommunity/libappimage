#include <memory>

#include "../traversal.h"

namespace appimage {
    namespace core {
        namespace impl {
            class traversal_fallback : public traversal {
                std::shared_ptr <std::istream> fileStream;
            public:
                traversal_fallback();

                void next() override;

                bool isCompleted() override;

                std::string getEntryName() override;

                void extract(const std::string& target) override;

                std::istream& read() override;
            };
        }
    }
}
