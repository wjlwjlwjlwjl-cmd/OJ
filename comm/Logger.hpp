#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> default_logger;

#define DEBUG(format, ...) default_logger->debug(std::string("{} + {}") + format, __FILE__, __LINE__, ##__VA_ARGS__);
#define ERROR(format, ...) default_logger->error(std::string("{} + {}") + format, __FILE__, __LINE__, ##__VA_ARGS__);

void init_logger()
{
    default_logger = std::make_shared<spdlog::logger>("default_logger");    
    default_logger->set_level(spdlog::level::level_enum::trace);
    default_logger->flush_on(spdlog::level::level_enum::trace);
    default_logger->set_pattern("[%n][%H:%M:%S][%t][%-8l] %v");
    default_logger->debug("{}", "{}", "hello", "hello");
}
