#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <ctime>

// Safe localtime wrapper for Windows and Linux
inline std::tm safe_localtime(std::time_t time) {
    std::tm tm_buf = {};
#ifdef _WIN32
    // Windows: use localtime_s (safe version)
    localtime_s(&tm_buf, &time);
#else
    // Linux/Mac: use localtime (works fine on Unix)
    const std::tm* result = std::localtime(&time);
    if (result) {
        tm_buf = *result;
    }
#endif
    return tm_buf;
}

class Logger {
public:
    enum class Level { DEBUG, INFO, WARNING, ERROR, CRITICAL };

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogLevel(Level level) {
        level_ = level;
    }

    template<typename... Args>
    void log(Level level, const std::string& message, Args&&... args) {
        if (level < level_) return;

        std::lock_guard<std::mutex> lock(mutex_);

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;

        // Use safe_localtime instead of localtime - NO WARNING!
        auto tm_time = safe_localtime(time);

        ss << std::put_time(&tm_time, "[%H:%M:%S")
            << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
        ss << "[" << levelToString(level) << "] ";
        ss << message;

        std::cout << ss.str() << std::endl;

        if (logFile_.is_open()) {
            logFile_ << ss.str() << std::endl;
            logFile_.flush();
        }
    }

private:
    Logger() : level_(Level::INFO) {
        logFile_.open("processing_framework.log", std::ios::app);
    }

    ~Logger() {
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static std::string levelToString(Level level) {
        switch (level) {
        case Level::DEBUG: return "DEBUG";
        case Level::INFO: return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR: return "ERROR";
        case Level::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
        }
    }

    Level level_;
    std::mutex mutex_;
    std::ofstream logFile_;
};

#define LOG_DEBUG(msg) Logger::getInstance().log(Logger::Level::DEBUG, msg)
#define LOG_INFO(msg) Logger::getInstance().log(Logger::Level::INFO, msg)
#define LOG_WARNING(msg) Logger::getInstance().log(Logger::Level::WARNING, msg)
#define LOG_ERROR(msg) Logger::getInstance().log(Logger::Level::ERROR, msg)
#define LOG_CRITICAL(msg) Logger::getInstance().log(Logger::Level::CRITICAL, msg)
