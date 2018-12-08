#pragma once

#include <string>
#include <fstream>
#include <vector>

namespace appimage {
    class MagicBytesChecker {
        std::ifstream input;
    public:
        MagicBytesChecker(const std::string& path);

        bool hasIso9660Signature();

        bool hasElfSignature();

        bool hasAppImageType1Signature();

        bool hasAppImageType2Signature();

    protected:
        bool hasSignatureAt(std::ifstream& input, std::vector<char>& signature, off_t offset);
    };
}
