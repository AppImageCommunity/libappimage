#pragma once
// system
#include <vector>
#include <string>

// libraries
#include <boost/filesystem.hpp>

/**
 * Private interface of the icon handler
 */
class IconHandlePriv {
public:
    explicit IconHandlePriv(const std::vector<char>& data) {}

    explicit IconHandlePriv(const std::string& path) {}

    virtual ~IconHandlePriv() = default;

    virtual int getOriginalSize() = 0;

    virtual int getSize() const = 0;

    virtual void setSize(int iconSize) = 0;

    virtual const std::string& getFormat() const = 0;

    virtual void save(const boost::filesystem::path& path, const std::string& targetFormat) = 0;
};
