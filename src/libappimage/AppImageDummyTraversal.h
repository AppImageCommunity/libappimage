#include "AppImageTraversal.h"

namespace AppImage {
    class AppImageDummyTraversal : public AppImageTraversal {
    public:
        void next() override;

        bool isCompleted() override;

        std::string getEntryName() override;

        void extract(const std::string& target) override;

        std::shared_ptr<std::istream> read() override;
    };
}

