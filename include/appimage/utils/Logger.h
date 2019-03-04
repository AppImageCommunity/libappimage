#pragma once

#include <memory>
#include <functional>

namespace appimage {
    namespace utils {
        enum class LogLevel {
            DEBUG = 0, INFO, WARNING, ERROR
        };

        typedef std::function<void(const utils::LogLevel& level, const std::string& message)> LogFunctionType;

        /**
         * Provides a global logger to be used in the libappimage context. It follows the singleton pattern.
         */
        class Logger {
        public:
            /**
             * @brief Set a custom logging function.
             * Allows to capture the libappimage log messages.
             *
             * @param logging function
             */
            void setFunction(const LogFunctionType& function);


            /**
             * Calls the default logging function or the one set by the user by means of "setFunction()".
             * @param level
             * @param message
             */
            void log(const utils::LogLevel& level, const std::string& message);

            /**
             * Utility function to directly generate a debug message.
             * @param message
             */
            static void debug(const std::string &message);

            /**
             * Utility function to directly generate a info message.
             * @param message
             */
            static void info(const std::string &message);

            /**
             * Utility function to directly generate a warning message.
             * @param message
             */
            static void warning(const std::string &message);

            /**
             * Utility function to directly generate a warning message.
             * @param message
             */
            static void error(const std::string &message);

            /**
             * @return an instance of Logger
             */
            static Logger* instance();

        private:
            // Singleton
            Logger();
            static std::unique_ptr<Logger> i;

            // PImpl
            class Priv;
            std::unique_ptr<Priv> d;
        };
    }
}
