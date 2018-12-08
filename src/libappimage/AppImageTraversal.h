#pragma once

#include <string>

namespace AppImage {

    class AppImageTraversal {
    public:
        virtual ~AppImageTraversal();

        virtual void next() = 0;

        virtual bool isCompleted() = 0;

        virtual std::string getEntryName() = 0;

        virtual void extract(const std::string& target) = 0;

        /**
         * Read file content.
         *
         * The returned istream becomes invalid every time next is called.
         * @return file content stream
         */
        virtual std::istream& read() = 0;
    };

}
