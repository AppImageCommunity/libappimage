// system
#include <fcntl.h>
#include <cstring>
#include <iostream>

// library
#include <archive.h>
#include <archive_entry.h>
#include <boost/filesystem.hpp>

// local
#include "appimage/core/AppImage.h"
#include "appimage/core/exceptions.h"
#include "appimage/appimage_shared.h"
#include "TraversalType1.h"
#include "StreambufType1.h"

using namespace std;
using namespace appimage::core::impl;

TraversalType1::TraversalType1(const std::string& path) : path(path) {
    clog << "Opening " << path << " as Type 1 AppImage" << endl;

    a = archive_read_new();
    archive_read_support_format_iso9660(a);

    if (archive_read_open_filename(a, path.c_str(), 10240) != ARCHIVE_OK)
        throw IOError(archive_error_string(a));

    completed = false;

    // Read first entry
    next();
}


TraversalType1::TraversalType1::~TraversalType1() {
    clog << "Closing " << path << endl;

    archive_read_close(a);
    archive_read_free(a);
}

void TraversalType1::next() {
    if (completed)
        return;

    readNextHeader();
    if (!completed) {
        readEntryData();

        // Skip the "." entry
        if (entryName == ".")
            next();

        /* Skip all but regular files and symlinks */
        if (entryType != PayloadEntryType::LINK && entryType != PayloadEntryType::REGULAR)
            next();
    }
}

bool TraversalType1::isCompleted() const { return completed; }

std::string TraversalType1::getEntryPath() const { return entryName; }

appimage::core::PayloadEntryType TraversalType1::getEntryType() const { return entryType; }

string TraversalType1::getEntryLinkTarget() const { return entryLink; }

void TraversalType1::extract(const std::string& target) {
    // create target parent dir
    auto parentPath = boost::filesystem::path(target).parent_path();
    boost::filesystem::create_directories(parentPath);

    // create file with user read and write permissions and only read permission for others and group
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int f = open(target.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);

    if (f == -1)
        throw FileSystemError("Unable to open file: " + target);

    // call the libarchive extract file implementation
    archive_read_data_into_fd(a, f);
    close(f);
}

istream& TraversalType1::read() {
    // create a new streambuf for reading the current entry
    auto tmpBuffer = new StreambufType1(a, 1024);

    // replace buffer in the istream
    entryIStream.rdbuf(tmpBuffer);

    // replace and drop the old buffer
    entryStreambuf.reset(tmpBuffer);

    return entryIStream;
}

void TraversalType1::readNextHeader() {
    int r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_EOF) {
        completed = true;
        return;
    }

    if (r != ARCHIVE_OK)
        throw IOError(archive_error_string(a));
}

void TraversalType1::readEntryData() {
    entryName = readEntryName();
    entryLink = readEntryLink();
    entryType = readEntryType();
}

appimage::core::PayloadEntryType TraversalType1::readEntryType() {
    // Hard links are reported by libarchive as regular files, this a workaround
    if (!entryLink.empty())
        return PayloadEntryType::LINK;

    auto entryType = archive_entry_filetype(entry);
    switch (entryType) {
        case AE_IFREG:
            return PayloadEntryType::REGULAR;
        case AE_IFLNK:
            return PayloadEntryType::LINK;
        case AE_IFDIR:
            return PayloadEntryType::DIR;
        default:
            return PayloadEntryType::UNKNOWN;
    }
}

std::string TraversalType1::readEntryLink() {
    auto symlink = archive_entry_symlink(entry);
    if (symlink)
        return symlink + 2;

    auto hardlink = archive_entry_hardlink(entry);
    if (hardlink)
        return hardlink + 2;

    return std::string();
}

std::string TraversalType1::readEntryName() {
    if (completed)
        return std::string();

    if (entry == nullptr)
        return std::string();

    const char* entryName = archive_entry_pathname(entry);
    if (entryName == nullptr)
        return string();

    // remove ./ prefix from entries names
    if (strncmp("./", entryName, 2) == 0)
        return entryName + 2;

    return entryName;
}
