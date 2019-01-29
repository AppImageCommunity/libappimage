#pragma once

// system
#include <memory>
#include <iterator>

// local
#include <appimage/core/Format.h>
#include <appimage/core/EntryType.h>

namespace appimage {
    namespace core {

        class AppImage;

        /**
         * A FilesIterator object provides a READONLY, SINGLE WAY, ONE PASS iterator over the files contained
         * in the AppImage pointed by <path>. Abstracts the users from the AppImage file payload format.
         *
         * READONLY: files inside the AppImage cannot  be modified.
         * SINGLE WAY: can't go backwards only forward.
         * ONE PASS: A new instance is required to re-traverse or the AppImage.
         */
        class FilesIterator : public std::iterator<std::input_iterator_tag, std::string> {
        public:
            /**
             * Create a FilesIterator for <appImage>
             * @param appImage
             * @throw AppImageReadError in case of error
             */
            explicit FilesIterator(const AppImage& appImage);

            /**
             * @return the type of the current file.
             */
            entry::Type type();

            /**
             * @return file path pointed by the iterator
             */
            std::string path();

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
             * Compare this iterator to <other>.
             * @param other
             * @return true of both are equal, false otherwise
             */
            bool operator==(const FilesIterator& other) const;

            /**
             * Compare this iterator to <other>.
             * @param other
             * @return true if are different, false otherwise
             */
            bool operator!=(const FilesIterator& other) const;

            /**
             * @return file path pointed by the iterator
             */
            std::string operator*();

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
            class Private;
            std::shared_ptr<Private> d;

            /**
             * Constructor used to create special representations of an iterator like the end state.
             * @param private data of the iterator
             */
            explicit FilesIterator(Private* d);
        };
    }
}
