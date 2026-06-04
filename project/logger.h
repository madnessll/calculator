#pragma once
#include <string>
#include <memory>

class Logger {
public:
    static Logger& getInstance();
    
    void info(const std::string& message);
    void error(const std::string& message);

private:
    Logger();
    ~Logger();
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    struct Impl;
    std::unique_ptr<Impl> impl_;
};