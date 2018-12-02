#include <archive.h>
#include <iostream>
#include <archive_entry.h>

#include "AppImage.h"
#include "Errors.h"
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

    return archive_entry_pathname(entry);
}

void AppImage::AppImageType1Traversal::next() {
    int r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_EOF){
        completed = true;
        return;
    }

    if (r != ARCHIVE_OK)
        throw AppImageReadError(archive_error_string(a));
}
