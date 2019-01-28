
// local
#include "Traversal.h"
namespace appimage {
    namespace core {
        bool Traversal::operator==(const Traversal& rhs) const {
            return getEntryName() == rhs.getEntryName() &&
                   getEntryType() == rhs.getEntryType() &&
                   getEntryLink() == rhs.getEntryLink();

        }

        bool Traversal::operator!=(const Traversal& rhs) const {}
    }
}
