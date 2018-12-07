#include <iostream>

#include <fcntl.h>
#include <archive.h>
#include <archive_entry.h>
#include <boost/filesystem.hpp>

#include "AppImage.h"
#include "AppImageErrors.h"
#include "AppImageType1Traversal.h"
#include "appimage_handler.h"
#include "appimage/appimage_shared.h"
#include "AppImageDummyStreamBuffer.h"
#include "AppImageFileStream.h"

using namespace std;

AppImage::AppImageType1Traversal::AppImageType1Traversal(const std::string& path) : path(path) {
    cerr << "Opening " << path << " as Type 1 AppImage" << endl;

    a = archive_read_new();
    archive_read_support_format_iso9660(a);
    if (archive_read_open_filename(a, path.c_str(), 10240) != ARCHIVE_OK)
        throw AppImageReadError(archive_error_string(a));

    completed = false;
}


AppImage::AppImageType1Traversal::~AppImageType1Traversal() {
    cerr << "Closing " << path << endl;

    archive_read_close(a);
    archive_read_free(a);
}

bool AppImage::AppImageType1Traversal::isCompleted() {
    return completed;
}

std::string AppImage::AppImageType1Traversal::getEntryName() {
    if (completed)
        return std::string();

    const char* entryName = archive_entry_pathname(entry);
    if (entryName == nullptr)
        return string();

    if (strncmp("./", entryName, 2) == 0)
        return entryName + 2;

    return entryName;
}

void AppImage::AppImageType1Traversal::next() {
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

void AppImage::AppImageType1Traversal::extract(const std::string& target) {
    boost::filesystem::path targetPath(path);
    boost::filesystem::create_directories(targetPath.parent_path());

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int f = open(target.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);

    if (f == -1)
        throw AppImageError("Unable to open file: " + target);

    archive_read_data_into_fd(a, f);
    close(f);
}

shared_ptr<istream> AppImage::AppImageType1Traversal::read() {
    auto dummyStreamBuffer = shared_ptr<streambuf>(new AppImageDummyStreamBuffer());
    auto istream = new AppImageFileStream(dummyStreamBuffer);

    return std::shared_ptr<std::istream>(istream);
}
