#pragma once

namespace appimage {
    namespace core {
        /**
         * Entry types known to the PayloadIterator
         */
        enum class PayloadEntryType {
            UNKNOWN = -1,   // another kind of entry, could be a special file
            REGULAR = 0,    // regular file
            DIR = 1,        // directory
            LINK = 2        // hard or symbolic link
        };
    }
}
