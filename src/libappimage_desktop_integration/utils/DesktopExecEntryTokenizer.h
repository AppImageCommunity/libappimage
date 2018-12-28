#pragma once

// system
#include <string>
#include <memory>

namespace appimage {
    namespace utils {
        /**
         * Allow to iterate over the Exec Desktop Entry Value respecting quoted sections according to
         * https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s07.html
         */
        class DesktopExecEntryTokenizer {
        public:
            explicit DesktopExecEntryTokenizer(const std::string& value);

            /**
             * Move to the next section.
             * @return true if a new section was found, false if the end of the string was reached
             */
            bool next();

            /**
             * @return current section value without quotes
             */
            std::string section() const;

            /**
             * @return current section start position
             */
            int sectionBegin() const;

            /**
             * @return current section size
             */
            int sectionSize() const;

        private:
            struct Priv;
            std::shared_ptr<Priv> d_ptr;
        };
    }
}
