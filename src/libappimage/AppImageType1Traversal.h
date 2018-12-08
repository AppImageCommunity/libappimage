#pragma once

#include <memory>
#include "AppImageTraversal.h"

namespace appimage {
    class AppImageType1Traversal : public AppImageTraversal {
        std::string path;
        struct archive* a = {nullptr};
        struct archive_entry* entry = {nullptr};

        std::shared_ptr<std::istream> appImageIStream;
        bool completed = false;
    public:
        explicit AppImageType1Traversal(const std::string& path);

        ~AppImageType1Traversal() override;

        void next() override;

        bool isCompleted() override;

        std::string getEntryName() override;

        void extract(const std::string& target) override;

        std::istream& read() override;
    };
}

