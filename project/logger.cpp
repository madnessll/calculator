#include "logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

struct Logger::Impl
{
    std::shared_ptr<spdlog::logger> logger;
};

Logger::Logger() : impl_(std::make_unique<Impl>())
{
    impl_->logger = spdlog::stdout_color_mt("calculator");
}

Logger::~Logger() = default;

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

void Logger::info(const std::string& message)
{
    impl_->logger->info(message);
}

void Logger::error(const std::string& message)
{
    impl_->logger->error(message);
}