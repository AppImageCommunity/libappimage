#include <iostream>

#include <appimage/appimage.h>
#include "AppImageErrors.h"
#include "AppImageType2Traversal.h"

using namespace std;

AppImage::AppImageType2Traversal::AppImageType2Traversal(std::string path) : path(path) {
    cerr << "Opening " << path << " as Type 2 AppImage" << endl;
    // The offset at which a squashfs image is expected
    ssize_t fs_offset = appimage_get_elf_size(path.c_str());

    if (fs_offset < 0)
        throw AppImageReadError("get_elf_size error");

    sqfs_err err = sqfs_open_image(&fs, path.c_str(), (size_t) fs_offset);

    if (err != SQFS_OK)
        throw AppImageReadError("sqfs_open_image error: " + path);

    root_inode = sqfs_inode_root(&fs);
    err = sqfs_traverse_open(&trv, &fs, root_inode);
    if (err != SQFS_OK)
        throw AppImageReadError("sqfs_traverse_open error");

}

AppImage::AppImageType2Traversal::~AppImageType2Traversal() {
    sqfs_traverse_close(&trv);

    cerr << "Closing " << path << " as Type 2 AppImage" << endl;
    sqfs_destroy(&fs);
}

void AppImage::AppImageType2Traversal::next() {
    sqfs_err err;
    if (!sqfs_traverse_next(&trv, &err))
        completed = true;

    if (err != SQFS_OK)
        throw AppImageReadError("sqfs_traverse_next error");
}

bool AppImage::AppImageType2Traversal::isCompleted() {
    return completed;
}

std::string AppImage::AppImageType2Traversal::getEntryName() {
    if (trv.path != nullptr)
        return trv.path;
    else
        return string();
}
