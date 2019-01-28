// local
#include <appimage/core/FilesIterator.h>
#include "core/impl/TraversalFallback.h"
#include "core/impl/TraversalType1.h"
#include "core/impl/TraversalType2.h"

using namespace appimage::core;

FilesIterator::FilesIterator(std::string path, FORMAT format) : last(new impl::TraversalFallback()) {
    switch (format) {
        case TYPE_1:
            priv = std::shared_ptr<Traversal>(new impl::TraversalType1(path));
            break;
        case TYPE_2:
            priv = std::shared_ptr<Traversal>(new impl::TraversalType2(path));
            break;
        default:
            // Behave properly in case of non-supported formats
            priv = std::shared_ptr<Traversal>(new impl::TraversalFallback());
            break;
    }
}

FilesIterator::FilesIterator(const std::shared_ptr<Traversal>& priv)
    : priv(priv), last(new impl::TraversalFallback()) {}

FilesIterator FilesIterator::begin() {
    if (!priv->isCompleted())
        priv->next();           // fetch the first element

    return *this;
}

FilesIterator FilesIterator::end() {
    return FilesIterator(last);
}

bool FilesIterator::operator!=(const FilesIterator& other) {
    if (priv != other.priv)
        return true;

    return priv->getEntryName() != other.priv->getEntryName();
}

FilesIterator& FilesIterator::operator++() {
    priv->next();

    if (priv->isCompleted())
        priv = last;   // set the iterator it its end state

    return *this;
}

std::string FilesIterator::operator*() {
    return priv->getEntryName();
}

entry::Type FilesIterator::type() {
    return priv->getEntryType();
}

void FilesIterator::extractTo(const std::string& target) {
    priv->extract(target);
}

std::istream& FilesIterator::read() {
    return priv->read();
}

std::string FilesIterator::link() {
    return priv->getEntryLink();
}

std::string FilesIterator::path() {
    return priv->getEntryName();
}