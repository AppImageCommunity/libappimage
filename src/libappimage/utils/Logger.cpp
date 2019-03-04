// system
#include <iostream>

// libraries
#include <boost/filesystem.hpp>

// local
#include <appimage/utils/Logger.h>


namespace appimage {
    namespace utils {
        std::unique_ptr<Logger> Logger::i = nullptr;

        class Logger::Priv {
        public:
            Priv() {
                // Default logging function
                logFunction = [](LogLevel level, const std::string& message) {
                    switch (level) {
                        case LogLevel::INFO:
                            std::clog << "INFO: ";
                            break;
                        case LogLevel::DEBUG:
                            std::clog << "DEBUG: ";
                            break;
                        case LogLevel::WARNING:
                            std::clog << "WARNING: ";
                            break;
                        case LogLevel::ERROR:
                            std::clog << "ERROR: ";
                            break;
                    }

                    std::clog << message << std::endl;
                };
            }

            LogFunctionType logFunction;
        };

        Logger::Logger() : d(new Priv) {}

        Logger* Logger::instance() {
            if (!i)
                i.reset(new Logger());

            return i.get();
        }

        void Logger::setFunction(const LogFunctionType& function) {
            d->logFunction = function;
        }

        void Logger::log(const utils::LogLevel& level, const std::string& message) {
            d->logFunction(level, message);
        }

        void Logger::debug(const std::string& message) {
            const auto i = instance();
            i->log(LogLevel::DEBUG, message);
        }

        void Logger::info(const std::string& message) {
            const auto i = instance();
            i->log(LogLevel::INFO, message);
        }

        void Logger::warning(const std::string& message) {
            const auto i = instance();
            i->log(LogLevel::WARNING, message);
        }

        void Logger::error(const std::string& message) {
            const auto i = instance();
            i->log(LogLevel::ERROR, message);
        }
    }
}
