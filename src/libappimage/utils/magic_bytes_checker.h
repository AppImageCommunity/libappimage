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
        class magic_bytes_checker {
            std::ifstream input;
        public:
            explicit magic_bytes_checker(const std::string& path);

            bool hasIso9660Signature();

            bool hasElfSignature();

            bool hasAppImageType1Signature();

            bool hasAppImageType2Signature();

        private:
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
