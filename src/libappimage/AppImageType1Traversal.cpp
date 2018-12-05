#include <archive.h>
#include <iostream>
#include <archive_entry.h>

#include "AppImage.h"
#include "AppImageErrors.h"
#include "AppImageType1Traversal.h"


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
    if (r == ARCHIVE_EOF){
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
