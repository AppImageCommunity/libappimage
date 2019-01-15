#pragma once

namespace appimage {
    namespace core {
        namespace entry {
            /**
             * Define file types known to the FileIterator
             */
            enum Type {
                UNKNOWN = -1,   // Not an AppImage file
                REGULAR = 0,     // Regular file
                DIR = 1,      // Directory
                LINK = 2       // Symlink
            };
        }
    }
}
