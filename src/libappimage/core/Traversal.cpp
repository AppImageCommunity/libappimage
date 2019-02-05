
// local
#include "Traversal.h"
namespace appimage {
    namespace core {
        bool Traversal::operator==(const Traversal& rhs) const {
            return getEntryPath() == rhs.getEntryPath() &&
                   getEntryType() == rhs.getEntryType() &&
                getEntryLinkTarget() == rhs.getEntryLinkTarget();

        }

        bool Traversal::operator!=(const Traversal& rhs) const {}
    }
}
