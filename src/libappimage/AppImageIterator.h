#pragma once

#include <memory>
#include <iterator>

#include "AppImageFormat.h"

namespace AppImage {
    class AppImageTraversal;

    typedef std::iterator<std::input_iterator_tag, std::string> AppImageBaseIterator;

    class AppImageIterator : public AppImageBaseIterator {
    public:
        AppImageIterator(std::string path, AppImage::Format format);

        bool operator!=(const AppImageIterator& other);

        std::string operator*();

        void extractTo(const std::string &target);

        /**
         * Read file content.
         *
         * The returned istream becomes invalid every time next is called.
         * @return file content stream
         */
        std::istream& read();

        AppImageIterator& operator++();

        AppImageIterator begin();

        AppImageIterator end();

    private:
        std::shared_ptr<AppImageTraversal> priv;
        std::shared_ptr<AppImageTraversal> last; // Represent the end state of the iterator

        AppImageIterator(const std::shared_ptr<AppImage::AppImageTraversal>& priv);
    };
}
