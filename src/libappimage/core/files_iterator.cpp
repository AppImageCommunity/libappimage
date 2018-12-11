// local
#include "files_iterator.h"
#include "impl/traversal_fallback.h"
#include "impl/traversal_type_1.h"
#include "impl/traversal_type_2.h"

using namespace appimage::core;

files_iterator::files_iterator(std::string path, FORMAT format) : last(new impl::traversal_fallback()) {
    switch (format) {
        case TYPE_1:
            priv = std::shared_ptr<traversal>(new impl::traversal_type_1(path));
            break;
        case TYPE_2:
            priv = std::shared_ptr<traversal>(new impl::traversal_type_2(path));
            break;
        default:
            // Behave properly in case of non-supported formats
            priv = std::shared_ptr<traversal>(new impl::traversal_fallback());
            break;
    }
}

files_iterator::files_iterator(const std::shared_ptr<traversal>& priv)
    : priv(priv), last(new impl::traversal_fallback()) {}

files_iterator files_iterator::begin() {
    if (!priv->isCompleted())
        priv->next();           // fetch the first element

    return *this;
}

files_iterator files_iterator::end() {
    return files_iterator(last);
}

bool files_iterator::operator!=(const files_iterator& other) {
    if (priv != other.priv)
        return true;

    return priv->getEntryName() != other.priv->getEntryName();
}

files_iterator& files_iterator::operator++() {
    priv->next();

    if (priv->isCompleted())
        priv = last;   // set the iterator it its end state

    return *this;
}

std::string files_iterator::operator*() {
    return priv->getEntryName();
}

void files_iterator::extractTo(const std::string& target) {
    priv->extract(target);
}

std::istream& files_iterator::read() {
    return priv->read();
}
