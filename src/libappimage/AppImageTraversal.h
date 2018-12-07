#pragma once

#include <string>
#include <boost/shared_ptr.hpp>
namespace AppImage {

    class AppImageTraversal {
    public:
        virtual ~AppImageTraversal();

        virtual void next() = 0;

        virtual bool isCompleted() = 0;

        virtual std::string getEntryName() = 0;

        virtual void extract(const std::string& target) = 0;

        virtual std::shared_ptr<std::istream> read() = 0;
    };

}
