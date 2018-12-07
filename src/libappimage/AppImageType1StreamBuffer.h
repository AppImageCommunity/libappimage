#pragma once

#include <vector>
#include <streambuf>

class AppImageType1StreamBuffer : public std::streambuf {
    unsigned long size;
    std::vector<char> buffer;
    struct archive* a = {nullptr};
protected:
public:
    AppImageType1StreamBuffer(archive* a, unsigned long size);

protected:
    int underflow() override;
};
