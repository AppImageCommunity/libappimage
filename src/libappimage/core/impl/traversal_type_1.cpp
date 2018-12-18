// system
#include <fcntl.h>
#include <cstring>
#include <iostream>

// library
#include <archive.h>
#include <archive_entry.h>
#include <boost/filesystem.hpp>

// local
#include "core/appimage.h"
#include "core/exceptions.h"
#include "core/file_istream.h"
#include "traversal_type_1.h"
#include "appimage/appimage_shared.h"
#include "streambuf_fallback.h"
#include "streambuf_type1.h"

using namespace std;
using namespace appimage::core::impl;

traversal_type_1::traversal_type_1(const std::string& path) : path(path) {
    clog << "Opening " << path << " as Type 1 AppImage" << endl;

    a = archive_read_new();
    archive_read_support_format_iso9660(a);
    if (archive_read_open_filename(a, path.c_str(), 10240) != ARCHIVE_OK)
        throw AppImageReadError(archive_error_string(a));

    completed = false;
}


traversal_type_1::traversal_type_1::~traversal_type_1() {
    clog << "Closing " << path << endl;

    archive_read_close(a);
    archive_read_free(a);
}

bool traversal_type_1::isCompleted() {
    return completed;
}

std::string traversal_type_1::getEntryName() {
    if (completed)
        return std::string();

    const char* entryName = archive_entry_pathname(entry);
    if (entryName == nullptr)
        return string();

    // remove ./ prefix from entries names
    if (strncmp("./", entryName, 2) == 0)
        return entryName + 2;

    return entryName;
}

void traversal_type_1::next() {
    int r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_EOF) {
        completed = true;
        return;
    }

    if (r != ARCHIVE_OK)
        throw AppImageReadError(archive_error_string(a));

    // Skip the "." entry
    const char* entryName = archive_entry_pathname(entry);
    if (strcmp(entryName, ".") == 0)
        next();
}

void traversal_type_1::extract(const std::string& target) {
    // create target parent dir
    auto parentPath = boost::filesystem::path(target).parent_path();
    boost::filesystem::create_directories(parentPath);

    // create file with user read and write permissions and only read permission for others and group
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int f = open(target.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);

    if (f == -1)
        throw AppImageError("Unable to open file: " + target);

    // call the libarchive extract file implementation
    archive_read_data_into_fd(a, f);
    close(f);
}

istream& traversal_type_1::read() {
    // create a new streambuf for reading the current entry
    auto streamBuffer = shared_ptr<streambuf>(new streambuf_type1(a, 1024));
    appImageIStream.reset(new file_istream(streamBuffer));

    return *appImageIStream.get();
}
