// system
#include <cstring>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <vector>

// local
#include "magic_bytes_checker.h"

using namespace appimage::utils;

magic_bytes_checker::magic_bytes_checker(const std::string& path) : input(path, std::ios_base::binary) {}

bool magic_bytes_checker::hasIso9660Signature() {
    /* Implementation of the signature matches expressed at https://www.garykessler.net/library/file_sigs.html
     * Signature: 43 44 30 30 31 	  	= "CD001"
     * ISO 	  	ISO-9660 CD Disc Image
     * This signature usually occurs at byte offset 32769 (0x8001),
     * 34817 (0x8801), or 36865 (0x9001).
     * More information can be found at MacTech or at ECMA.
     */

    if (!input.fail()) {
        off_t positions[] = {32769, 34817, 36865};
        std::vector<char> signature = {'C', 'D', '0', '0', '1'};
        for (int i = 0; i < 3; i++)
            if (hasSignatureAt(input, signature, positions[i]))
                return true;
    }

    return false;
}

bool magic_bytes_checker::hasElfSignature() {
    if (!input.fail()) {
        // check magic hex 0x7f 0x45 0x4c 0x46 at offset 0
        std::vector<char> signature = {0x7f, 0x45, 0x4c, 0x46};
        if (hasSignatureAt(input, signature, 0))
            return true;
    }
    return false;
}

bool magic_bytes_checker::hasAppImageType1Signature() {
    if (!input.fail()) {
        // check magic hex 0x414901 at offset 8
        std::vector<char> signature = {0x41, 0x49, 0x01};
        if (hasSignatureAt(input, signature, 8))
            return true;
    }
    return false;
}

bool magic_bytes_checker::hasAppImageType2Signature() {
    if (!input.fail()) {
        // check magic hex 0x414902 at offset 8
        std::vector<char> signature = {0x41, 0x49, 0x02};
        if (hasSignatureAt(input, signature, 8))
            return true;
    }
    return false;
}

bool magic_bytes_checker::hasSignatureAt(std::ifstream& input, std::vector<char>& signature, off_t offset) {
    try {
        // move to the right offset in the file
        input.seekg(offset, std::ios_base::beg);

        // compare file content with the signature
        if (equal(signature.begin(), signature.end(), std::istream_iterator<char>(input)))
            return true;

    } catch (const std::ios_base::failure&) {}

    return false;
}
