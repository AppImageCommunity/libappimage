#pragma once

// system
#include <memory>
#include <iterator>

// local
#include <appimage/core/Format.h>
#include <appimage/core/EntryType.h>

namespace appimage {
    namespace core {

        // Forward declaration required to use the class as an opaque pointer at FilesIterator
        // Derived classes implement the real Traversal algorithms over the AppImage files.
        class Traversal;

        /**
         * An files_iterator object provides a READONLY, SINGLE WAY, ONE PASS iterator over the files contained
         * in the AppImage pointed by <path>. Abstracts the users from the AppImage file payload format.
         *
         * READONLY: files inside the AppImage cannot  be modified.
         * SINGLE WAY: can't go backwards only forward.
         * ONE PASS: A new instance is required to re-traverse or the AppImage.
         */
        class FilesIterator : public std::iterator<std::input_iterator_tag, std::string> {
        public:
            /**
             * Create a files_iterator for <path> assuming that the file is an AppImage of <format>
             * @param path
             * @param format
             * @throw AppImageReadError in case of error
             */
            FilesIterator(std::string path, FORMAT format);

            /**
             * Compare two iterators according to their paths
             * @param other
             * @return true if iterators are different
             */
            bool operator!=(const FilesIterator& other);

            /**
             * @return file path pointed by the iterator
             */
            std::string operator*();

            /**
             * @return file path pointed by the iterator
             */
            std::string path();

            /**
             * @return the type of the current file.
             */
            entry::Type type();

            /**
             * @return file link path if it's a LINK type file. Otherwise returns an empty string.
             */
            std::string link();

            /**
             * Extracts the file to the <target> path. Supports raw files, symlinks and directories.
             * Parent target dir is created if not exists.
             * @param target
             */
            void extractTo(const std::string& target);

            /**
             * Read file content. Symbolic links will be resolved.
             *
             * The returned istream becomes invalid every time next is called.
             * @return file content stream
             */
            std::istream& read();

            /**
             * Move iterator to the next file.
             * @return current file_iterator
             */
            FilesIterator& operator++();

            /**
             * Represents the begin of the iterator. Will  always point to the current iterator.
             * @return current file_iterator
             */
            FilesIterator begin();

            /**
             * Represent the end of the iterator. Will  always point to an invalid iterator.
             * @return invalid file_iterator
             */
            FilesIterator end();

        private:
            std::shared_ptr<Traversal> priv; // Current Traversal status
            std::shared_ptr<Traversal> last; // Represent the end state of the iterator

            FilesIterator(const std::shared_ptr<Traversal>& priv);
        };
    }
}