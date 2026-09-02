#pragma once
#include <memory>
#include <string>

class Logger
{
  public:
    static Logger& getInstance();

    void info(const std::string& message);
    void error(const std::string& message);

  private:
    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    struct Impl;
    std::unique_ptr<Impl> impl_;
};