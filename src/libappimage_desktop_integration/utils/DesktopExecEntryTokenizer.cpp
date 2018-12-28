// system
#include <sstream>

// local
#include "DesktopExecEntryTokenizer.h"

namespace appimage {
    namespace utils {
        struct DesktopExecEntryTokenizer::Priv {
            std::string value;
            unsigned long begin = 0;
            unsigned long size = 0;

            std::string section;
        };

        DesktopExecEntryTokenizer::DesktopExecEntryTokenizer(const std::string& value) : d_ptr(new Priv()) {
            d_ptr->value = value;
        }

        bool DesktopExecEntryTokenizer::next() {
            if ((d_ptr->begin + d_ptr->size) >= d_ptr->value.size())
                return false;

            // entries sections might delimited by a quote char (") or by blank spaces

            bool sectionCompleted = false;
            bool quotedSection = false;
            bool escapedChar = false;
            unsigned long i = d_ptr->begin + d_ptr->size;

            // ignore white spaces at the begin
            while (i < d_ptr->value.size() && d_ptr->value[i] == ' ')
                i++;

            d_ptr->begin = i;
            std::stringstream newSection;

            for (; i < d_ptr->value.size() && !sectionCompleted; i++) {
                const char& c = d_ptr->value[i];

                if (escapedChar) {
                    escapedChar = false;
                    newSection << c;
                } else {
                    // quoted section begin or end
                    if (c == '\"' && !escapedChar)
                        quotedSection = !quotedSection;

                    if (c == '\\')
                        escapedChar = true;

                    // a blank space points the end of a non quoted section
                    if (c == ' ' && !quotedSection)
                        sectionCompleted = true;

                    // append char to new section
                    if (!sectionCompleted)
                        newSection << c;
                }
            }

//          update pointers
            d_ptr->section = newSection.str();
            d_ptr->size = i - d_ptr->begin;

            // don't count the space in the section size
            if (i < d_ptr->value.size())
                d_ptr->size--;

            return true;
        }

        std::string DesktopExecEntryTokenizer::section() const {
            return d_ptr->section;
        }

        int DesktopExecEntryTokenizer::sectionBegin() const {
            return d_ptr->begin;
        }

        int DesktopExecEntryTokenizer::sectionSize() const {
            return d_ptr->size;
        }
    }
}
