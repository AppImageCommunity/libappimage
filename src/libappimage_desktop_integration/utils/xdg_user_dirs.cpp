// system
#include <cstdlib>

// local
#include "xdg_user_dirs.h"
#include "environment.h"

namespace appimage {
    namespace utils {
        std::string xdg_user_dirs::data() {
            environment e;
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

