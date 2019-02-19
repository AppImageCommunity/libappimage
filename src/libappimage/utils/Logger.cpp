// libraries
#include <boost/filesystem.hpp>

// local
#include "Logger.h"


namespace appimage {
    namespace utils {
        Logger::Logger(const std::string& prefix, std::ostream& ostream) noexcept
            : logPrefix(prefix), ostream(ostream), loglevel(LogLevel::DEBUG) {}

        void Logger::setLoglevel(LogLevel loglevel) {
            Logger::loglevel = loglevel;
        }

        Logger::Log Logger::debug() const {
            if (loglevel <= LogLevel::DEBUG) {
                ostream << logPrefix << ": DEBUG ";
                return Logger::Log(ostream);
            } else {
                return Logger::Log();
            }
        }

        Logger::Log Logger::info() const {
            if (loglevel <= LogLevel::INFO) {
                ostream << logPrefix << ": INFO ";
                return Logger::Log(ostream);
            } else {
                return Logger::Log();
            }
        }

        Logger::Log Logger::warning() const {
            if (loglevel <= LogLevel::INFO) {
                ostream << logPrefix << ": WARNING ";
                return Logger::Log(ostream);
            } else {
                return Logger::Log();
            };
        }

        Logger::Log Logger::error() const {
            ostream << logPrefix << ": ERROR ";
            return Logger::Log(ostream);
        }
    }
}
