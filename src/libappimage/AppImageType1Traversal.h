#pragma once

#include <memory>
#include "AppImageTraversal.h"

namespace AppImage {
    class AppImageType1Traversal : public AppImageTraversal {
        std::string path;
        struct archive* a = {nullptr};
        struct archive_entry* entry = {nullptr};

        bool completed = false;
    public:
        explicit AppImageType1Traversal(const std::string& path);

        ~AppImageType1Traversal() override;

        void next() override;

        bool isCompleted() override;

        std::string getEntryName() override;
    };
}

