// system
#include <iostream>
#include <filesystem>

// local
#include "Logger.h"


namespace appimage {
    namespace utils {
        class Logger::Priv {
        public:
            // singleton
            static std::unique_ptr<Logger> i;

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

            log_callback_t logFunction;
        };

        std::unique_ptr<Logger> Logger::Priv::i = nullptr;

        Logger::Logger() : d(new Priv) {}

        Logger* Logger::getInstance() {
            if (!Priv::i)
                Priv::i.reset(new Logger());

            return Priv::i.get();
        }

        void Logger::setCallback(const log_callback_t& callback) {
            d->logFunction = callback;
        }

        void Logger::log(const utils::LogLevel& level, const std::string& message) {
            d->logFunction(level, message);
        }

        void Logger::debug(const std::string& message) {
            const auto i = getInstance();
            i->log(LogLevel::DEBUG, message);
        }

        void Logger::info(const std::string& message) {
            const auto i = getInstance();
            i->log(LogLevel::INFO, message);
        }

        void Logger::warning(const std::string& message) {
            const auto i = getInstance();
            i->log(LogLevel::WARNING, message);
        }

        void Logger::error(const std::string& message) {
            const auto i = getInstance();
            i->log(LogLevel::ERROR, message);
        }

        void setLoggerCallback(const log_callback_t& callback) {
            Logger::getInstance()->setCallback(callback);
        }
    }
}
