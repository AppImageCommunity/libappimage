#include "AppImageIterator.h"
#include "AppImageType1Traversal.h"
#include "AppImageDummyTraversal.h"
#include "AppImageType2Traversal.h"


appimage::AppImageIterator::AppImageIterator(std::string path, appimage::Format format) : last(
    new AppImageDummyTraversal()) {
    switch (format) {
        case Type1:
            priv = std::shared_ptr<AppImageTraversal>(new AppImageType1Traversal(path));
            break;
        case Type2:
            priv = std::shared_ptr<AppImageTraversal>(new AppImageType2Traversal(path));
            break;
        default:
            priv = std::shared_ptr<AppImageTraversal>(new AppImageDummyTraversal());
            break;
    }
}

appimage::AppImageIterator::AppImageIterator(const std::shared_ptr<appimage::AppImageTraversal>& priv)
    : priv(priv), last(new AppImageDummyTraversal()) {}

appimage::AppImageIterator appimage::AppImageIterator::begin() {
    if (!priv->isCompleted())
        priv->next();

    return *this;
}

appimage::AppImageIterator appimage::AppImageIterator::end() {
    return AppImageIterator(last);
}

bool appimage::AppImageIterator::operator!=(const appimage::AppImageIterator& other) {
    if (priv != other.priv)
        return true;

    return priv->getEntryName() != other.priv->getEntryName();
}

appimage::AppImageIterator& appimage::AppImageIterator::operator++() {
    priv->next();

    if (priv->isCompleted())
        priv = last;

    return *this;
}

std::string appimage::AppImageIterator::operator*() {
    return priv->getEntryName();
}

void appimage::AppImageIterator::extractTo(const std::string& target) {
    priv->extract(target);
}

std::istream& appimage::AppImageIterator::read() {
    return priv->read();
}
