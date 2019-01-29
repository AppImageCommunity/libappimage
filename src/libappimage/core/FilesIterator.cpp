// system
#include <sstream>

// local
#include <appimage/core/FilesIterator.h>
#include <appimage/core/AppImage.h>
#include "core/impl/TraversalType1.h"
#include "core/impl/TraversalType2.h"

namespace appimage {
    namespace core {

        /**
         * @brief Representation of the private state of the iterator.
         *
         * A Traversal class is used to traverse the files and directories inside the AppImage payload. The required
         * Traversal derivative is instantiated on demand by the constructor.
         *
         * The "end state" of the iterator is represented by a nullptr traversal.
         *
         * The major part of this class methods are proxies over the Traversal class. Therefore they behave the same. If
         * the traversal reach the "end state" those methods will return a default value to keep the integrity of the
         * iterator.
         */
        class FilesIterator::Private {
            AppImage appImage;

            // to be used by the read method when the end of the traversal is reached
            std::stringstream emptyStream;

            // Real Traversal implementation
            std::shared_ptr<Traversal> traversal;
        public:
            /**
             * Initialized the Private class with required traversal derivative if <atEnd> is false.
             * @param appImage
             * @param atEnd determine if a end state iterator should be created
             */
            explicit Private(const AppImage& appImage, bool atEnd = false);

            // Creating copies of this object is not allowed
            Private(Private& other) = delete;

            // Creating copies of this object is not allowed
            Private& operator=(Private& other) = delete;

            // Move constructor
            Private(Private&& other) noexcept;

            // Move assignment operator
            Private& operator=(Private&& other) noexcept;

            /**
             * Compare Private data according to the AppImage they point to and to the traversal instance.
             * @param rhs
             * @return
             */
            bool operator==(const Private& rhs) const;

            bool operator!=(const Private& rhs) const;

            bool isCompleted() { return traversal == nullptr; }

            void next();

            entry::Type type() { return isCompleted() ? entry::UNKNOWN : traversal->getEntryType(); }

            std::string entryName() { return isCompleted() ? std::string() : traversal->getEntryName(); }

            std::string entryLink() { return isCompleted() ? std::string() : traversal->getEntryLink(); }

            void extractTo(const std::string& target) { if (!isCompleted()) traversal->extract(target); }

            std::istream& read() { return isCompleted() ? emptyStream : traversal->read(); }

            Private* beginState() { return new Private(appImage); }

            Private* endState() { return new Private(appImage, true); }
        };

        FilesIterator::FilesIterator(const AppImage& appImage) : d(new Private(appImage)) {}

        FilesIterator::FilesIterator(FilesIterator&& other) noexcept { d = other.d; }

        FilesIterator& FilesIterator::operator=(FilesIterator&& other) noexcept {
            d = other.d;
            return *this;
        }

        entry::Type FilesIterator::type() { return d->type(); }

        std::string FilesIterator::path() { return d->entryName(); }

        std::string FilesIterator::link() { return d->entryLink(); }

        void FilesIterator::extractTo(const std::string& target) { d->extractTo(target); }

        std::istream& FilesIterator::read() { return d->read(); }

        std::string FilesIterator::operator*() { return d->entryName(); }

        FilesIterator& FilesIterator::operator++() {
            // move to the next entry in the traversal
            d->next();

            return *this;
        }

        FilesIterator FilesIterator::begin() { return FilesIterator(d->beginState()); }

        FilesIterator FilesIterator::end() { return FilesIterator(d->endState()); }

        FilesIterator::FilesIterator(FilesIterator::Private* d) : d(d) {}

        bool FilesIterator::operator==(const FilesIterator& other) const { return *d == *(other.d); }

        bool FilesIterator::operator!=(const FilesIterator& other) const { return !(other == *this); }

        /**
         * FilesIterator Private methods
         */
        FilesIterator::Private::Private(const AppImage& appImage, bool atEnd) : appImage(appImage) {
            // only initialize if the iterator is not in the "end state"
            if (!atEnd) {
                switch (appImage.getFormat()) {
                    case TYPE_1:
                        traversal = std::shared_ptr<Traversal>(new impl::TraversalType1(appImage.getPath()));
                        break;
                    case TYPE_2:
                        traversal = std::shared_ptr<Traversal>(new impl::TraversalType2(appImage.getPath()));
                        break;
                    default:
                        break;
                }
            }
        }

        bool FilesIterator::Private::operator==(const FilesIterator::Private& rhs) const {
            return appImage == rhs.appImage &&
                   traversal == rhs.traversal;
        }

        bool FilesIterator::Private::operator!=(const FilesIterator::Private& rhs) const {
            return !(rhs == *this);
        }

        void FilesIterator::Private::next() {
            // move forward only if we haven't reached the end
            if (traversal) {
                traversal->next();

                // release traversal instance when completed in order to match with the "end state"
                if (traversal->isCompleted())
                    traversal.reset();
            }
        }

        FilesIterator::Private::Private(FilesIterator::Private&& other) noexcept : appImage(other.appImage),
                                                                                   traversal(other.traversal) {}

        FilesIterator::Private& FilesIterator::Private::operator=(FilesIterator::Private&& other) noexcept {
            appImage = other.appImage;
            traversal = other.traversal;
            return *this;
        }
    }
}
