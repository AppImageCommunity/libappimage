#pragma once

#include <memory>
#include <iterator>

#include "format.h"

namespace appimage {
    namespace core {

        class traversal;

        typedef std::iterator<std::input_iterator_tag, std::string> appimage_base_iterator;

        class files_iterator : public appimage_base_iterator {
        public:
            files_iterator(std::string path, FORMAT format);

            bool operator!=(const files_iterator& other);

            std::string operator*();

            void extractTo(const std::string& target);

            /**
             * Read file content. Symbolic links will be resolved.
             *
             * The returned istream becomes invalid every time next is called.
             * @return file content stream
             */
            std::istream& read();

            files_iterator& operator++();

            files_iterator begin();

            files_iterator end();

        private:
            std::shared_ptr<traversal> priv;
            std::shared_ptr<traversal> last; // Represent the end state of the iterator

            files_iterator(const std::shared_ptr<traversal>& priv);
        };
    }
}
