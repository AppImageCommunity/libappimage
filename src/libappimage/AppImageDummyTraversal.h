#include "AppImageTraversal.h"

namespace AppImage {
    class AppImageDummyTraversal : public AppImageTraversal {
    public:
        void next() override;

        bool isCompleted() override;

        std::string getEntryName() override;
    };
}

