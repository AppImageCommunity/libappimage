#include <memory>
#include "AppImageTraversal.h"

namespace appimage {
    class AppImageDummyTraversal : public AppImageTraversal {
        std::shared_ptr<std::istream> fileStream;
    public:
        void next() override;

        bool isCompleted() override;

        std::string getEntryName() override;

        void extract(const std::string& target) override;

        std::istream& read() override;
    };
}

