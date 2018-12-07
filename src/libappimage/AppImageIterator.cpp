#include "AppImageIterator.h"
#include "AppImageType1Traversal.h"
#include "AppImageDummyTraversal.h"
#include "AppImageType2Traversal.h"


AppImage::AppImageIterator::AppImageIterator(std::string path, AppImage::Format format) : last(
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

AppImage::AppImageIterator::AppImageIterator(const std::shared_ptr<AppImage::AppImageTraversal>& priv)
    : priv(priv), last(new AppImageDummyTraversal()) {}

AppImage::AppImageIterator AppImage::AppImageIterator::begin() {
    if (!priv->isCompleted())
        priv->next();

    return *this;
}

AppImage::AppImageIterator AppImage::AppImageIterator::end() {
    return AppImageIterator(last);
}

bool AppImage::AppImageIterator::operator!=(const AppImage::AppImageIterator& other) {
    if (priv != other.priv)
        return true;

    return priv->getEntryName() != other.priv->getEntryName();
}

AppImage::AppImageIterator& AppImage::AppImageIterator::operator++() {
    priv->next();

    if (priv->isCompleted())
        priv = last;

    return *this;
}

std::string AppImage::AppImageIterator::operator*() {
    return priv->getEntryName();
}

void AppImage::AppImageIterator::extractTo(const std::string& target) {
    priv->extract(target);
}

std::shared_ptr<std::istream> AppImage::AppImageIterator::read() {
    return priv->read();
}
