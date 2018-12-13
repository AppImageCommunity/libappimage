// system
#include <sstream>
#include <fcntl.h>
#include <string>

extern "C" {
#include <sys/stat.h>
#include <unistd.h>
// libraries

#include <glib.h>
#include <glib/gstdio.h>
}

// local
#include "core/exceptions.h"
#include "filesystem.h"

namespace appimage {
    namespace utils {
        namespace filesystem {
            std::string parentPath(const std::string& path) {
                // find last directory separator
                long i = path.rfind('/');

                // directory separators found
                if (i == std::string::npos)
                    return ".";

                // remove repeated separators
                while (i > 0 && path[i - 1] == '/')
                    i--;

                // if the only '/' is at the beginning of the string take it as the filesystem root
                if (i == 0)
                    return "/";

                // assume that the parent path was found if still not in the end of the string
                if (i > 0)
                    return path.substr(0, i);

                return ".";
            }

            void createDirectories(const std::string& path) {
                for (int i = 0; i < path.length(); i++) {
                    if (path[i] == '/') {
                        auto subDir = path.substr(0, i + 1);
                        if (access(subDir.c_str(), F_OK) != 0) {
                            if (mkdir(subDir.c_str(), 0777) == -1)
                                throw appimage::core::AppImageError("mkdir error " + path);
                        }
                    }
                }

                if (access(path.c_str(), F_OK) != 0) {
                    if (mkdir(path.c_str(), 0777) == -1)
                        throw appimage::core::AppImageError("mkdir error " + path);
                }
            }

            bool copyFile(const char* source, const char* target) {
                char* target_dir = g_path_get_dirname(target);
                int res = g_mkdir_with_parents(target_dir, S_IRWXU);
                g_free(target_dir);

                if (res == -1)
                    return false;

                int fin = open(source, O_RDONLY);
                int fout = open(target, O_WRONLY | O_CREAT, S_IRWXU);

                if (fin == -1 || fout == -1)
                    return false;

                const size_t BUF_SIZE = 1024;
                char buf[BUF_SIZE];
                size_t bs_read;
                while ((bs_read = static_cast<size_t>(read(fin, &buf, BUF_SIZE))) != 0)
                    write(fout, &buf, bs_read);

                close(fin);
                close(fout);

                return true;
            }

            std::string createTempDir(const std::string& baseName) {
                std::string result;
                GError* error = NULL;
                std::string template_name = baseName + "-XXXXXX";
                char* path = g_dir_make_tmp(template_name.c_str(), &error);
                if (error) {
                    g_warning("%s", error->message);
                    g_error_free(error);
                } else {
                    result = path;
                    free(path);
                }

                return result;
            }

            void removeRecursive(const std::string& path) {
                GDir* tempdir = NULL;
                GError* error;
                tempdir = g_dir_open(path.c_str(), 0, &error);
                if (!tempdir) {
                    g_warning("%s\n", error->message);
                    g_error_free(error);
                    return;
                }

                const char* entry;
                while ((entry = g_dir_read_name(tempdir)) != NULL) {
                    char* full_path = g_strjoin("/", path.c_str(), entry, NULL);
                    if (g_file_test(full_path, G_FILE_TEST_IS_DIR)) {
                        removeRecursive(full_path);
                    } else
                        g_remove(full_path);

                    free(full_path);
                }

                g_rmdir(path.c_str());
                g_dir_close(tempdir);
            }
        }
    }
}
