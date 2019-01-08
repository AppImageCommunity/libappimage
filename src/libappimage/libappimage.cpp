/**
 * Implementation of the C interface functions
 */
// system
#include <cstring>
#include <sstream>

// local
#include <appimage/core/AppImage.h>

using namespace appimage::core;

extern "C" {

/* Check if a file is an AppImage. Returns the image type if it is, or -1 if it isn't */
int appimage_get_type(const char* path, bool verbose) {
    try {
        AppImage appImage(path);
        return appImage.getFormat();
    } catch (...) {
        return -1;
    }
}

ssize_t appimage_get_elf_size(const char* fname) {
    try {
        AppImage appImage(fname);
        return appImage.getPayloadOffset();
    } catch (...) {}

    return 0;
}


char** appimage_list_files(const char* path) {
    char** result = nullptr;
    try {
        AppImage appImage(path);

        std::vector<std::string> files;
        for (auto itr = appImage.files(); itr != itr.end(); ++itr)
            if (!(*itr).empty())
                files.emplace_back(*itr);


        result = static_cast<char**>(malloc(sizeof(char*) * (files.size() + 1)));
        for (int i = 0; i < files.size(); i++)
            result[i] = strdup(files[i].c_str());

        result[files.size()] = nullptr;
    } catch (...) {
        // Create empty string list
        result = static_cast<char**>(malloc(sizeof(char*)));
        result[0] = nullptr;

        return result;
    }

    return result;
}

void appimage_string_list_free(char** list) {
    for (char** ptr = list; ptr != NULL && *ptr != NULL; ptr++)
        free(*ptr);

    free(list);
}

bool
appimage_read_file_into_buffer_following_symlinks(const char* appimage_file_path, const char* file_path, char** buffer,
                                                  unsigned long* buf_size) {
    *buffer = nullptr;
    *buf_size = 0;

    try {
        AppImage appImage(appimage_file_path);

        std::vector<char> data;

        bool found = false;
        for (auto itr = appImage.files(); itr != itr.end(); ++itr)
            if (*itr == file_path) {
                data = std::vector<char>(std::istreambuf_iterator<char>(itr.read()), std::istreambuf_iterator<char>());
                found = true;
                break;
            }

        if (!found)
            return false;

        *buffer = static_cast<char*>(malloc(sizeof(char) * data.size()));
        memcpy(*buffer, data.data(), data.size());

        *buf_size = data.size();

        return true;
    } catch (...) {
        return false;
    }
}


}
