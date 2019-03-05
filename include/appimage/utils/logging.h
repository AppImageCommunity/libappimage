#pragma once

#include <string>
#include <functional>

namespace appimage {
    namespace utils {
        enum class LogLevel {
            DEBUG = 0, INFO, WARNING, ERROR
        };

        typedef std::function<void(const utils::LogLevel& level, const std::string& message)> log_callback_t;

        /**
         * @brief Set a custom logging function.
         * Allows to capture the libappimage log messages.
         *
         * @param logging function callback
         */
        void setLoggerCallback(const log_callback_t& callback);
    }
}
