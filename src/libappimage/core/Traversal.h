#pragma once

#include <string>

namespace appimage {
    namespace core {
        /**
         * Abstract representation of an AppImage traversal operation. Serves as extension point for the
         * <files_iterator> class. Has the following restrictions:
         *
         * - READONLY: files inside the AppImage cannot  be modified.
         * - SINGLE WAY: can't go backwards only forward.
         * - ONE PASS: A new instance is required to re-traverse or the AppImage.
         * - NO ORDER: There is no warranty that the traversal will follow a given order.
         */
        class Traversal {
        public:
            virtual ~Traversal() = default;

            /**
             * Move to the next entry in the AppImage.
             */
            virtual void next() = 0;

            /**
             * @return true if the end of the traversal was reached, false otherwise
             */
            virtual bool isCompleted() = 0;

            /**
             * @return name of the file entry inside the AppImage
             */
            virtual std::string getEntryName() = 0;

            /**
             * Extracts the file to the <target> path. Supports raw files, symlinks and directories.
             * Parent target dir is created if not exists.
             * @param target path the file should be extracted
             */
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
}
