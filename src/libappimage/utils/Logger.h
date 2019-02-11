#pragma once

// system
#include <array>
#include <string>
#include <iostream>

namespace appimage {
    namespace utils {
        enum class LogLevel {
            DEBUG = 0, INFO, WARNING, ERROR
        };

        class Logger {
        public:
            explicit Logger(const std::string& prefix, std::ostream& ostream);

            class Log;

            Log debug() const;

            Log info() const;

            Log warning() const;

            Log error() const;

            void setLoglevel(LogLevel loglevel);

        private:
            LogLevel loglevel;
            std::ostream& ostream;
            std::string logPrefix;
        };

        class Logger::Log {
        public:
            template<class T>
            Log& operator<<(const T& item) {
                if (!sink)
                    stream << item;

                return *this;
            }

            /**
             * Forward std::ostream operators
             * @param f
             * @return
             */
            Log& operator<<(std::ostream& (* f)(std::ostream&)) {
                if (!sink)
                    stream << f;

                return *this;
            }

        private:
            friend class Logger;

            explicit Log() : stream(std::clog), sink(true) {}

            explicit Log(std::ostream& ostream) : stream(ostream) {}

            // Control where the message should be ignored
            bool sink = false;
            std::ostream& stream;
        };
    }
}
