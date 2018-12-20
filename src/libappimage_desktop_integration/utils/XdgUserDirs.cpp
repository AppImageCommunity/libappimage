// system
#include <cstdlib>

// local
#include "XdgUserDirs.h"
#include "Environment.h"

namespace appimage {
    namespace utils {
        std::string XdgUserDirs::data() {
            Environment e;
            auto dataHomeVal = e["XDG_DATA_HOME"];
            if (!dataHomeVal.empty())
                return dataHomeVal;
            else {
                // Fallback to $HOME/.local/share
                auto homeVal = e["HOME"];
                if (homeVal.empty())
                    return std::string();
                else
                    return homeVal + "/.local/share";
            }
        }
    }
}

