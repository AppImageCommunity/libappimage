#pragma once

namespace appimage {
    namespace core {
        /**
         * The AppImage format determines how an AppImage is represented on disk. See the link below for more details
         * https://github.com/AppImage/AppImageSpec/blob/master/draft.md#image-format
         */
        enum FORMAT {
            INVALID = -1,   // Not an AppImage file
            LEGACY = 0,     // portable binaries that look and behave like AppImages but do not follow the standard
            TYPE_1 = 1,      // https://github.com/AppImage/AppImageSpec/blob/master/draft.md#type-1-image-format
            TYPE_2 = 2       // https://github.com/AppImage/AppImageSpec/blob/master/draft.md#type-2-image-format
        };
    }
}
