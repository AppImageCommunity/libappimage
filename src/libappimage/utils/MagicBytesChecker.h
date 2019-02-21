#pragma once

// system
#include <string>
#include <fstream>
#include <vector>

namespace appimage {
    namespace utils {
        /**
         * Allows the verification of magic bytes at in a given file.
         */
        class MagicBytesChecker {
        public:
            explicit MagicBytesChecker(const std::string& path);

            bool hasIso9660Signature();

            bool hasElfSignature();

            bool hasAppImageType1Signature();

            bool hasAppImageType2Signature();

        private:
            std::ifstream input;

            /**
             * Verify if the input matches at <offset> with <signature>
             * @param input
             * @param signature
             * @param offset
             * @return true if there is a match, flase otherwise
             */
            bool hasSignatureAt(std::ifstream& input, std::vector<char>& signature, off_t offset);
        };
    }

}
