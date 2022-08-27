#pragma once

// system
#include <filesystem>

class TemporaryDirectory {
public:
    /**
     * Create temporary directory in the system's temporary directory with an optional prefix.
     * The base directory may be changed using the $TEMPDIR environment variable.
     * @param prefix prefix to add to the temporary directory's name
     */
    explicit TemporaryDirectory(const std::string& prefix = "");

    /**
     * Cleans up the temporary directory from the filesystem.
     */
    ~TemporaryDirectory();

    std::filesystem::path path() const;

private:
    std::filesystem::path _path;
};
