#include "Logger.hpp"
#include <ctime>

Logger::Logger() {}
Logger::~Logger() {}

void Logger::log(LOG_LEVEL level, const std::string message) throw() {
  const std::time_t curr_time = std::time(0);
  std::string result;
  char timestamp[21] = {0};

  switch (level) {
  case (LOG_INFO):
    result.append("[INFO] ");
    break;
  case (LOG_WARNING):
    result.append(COLOR_YELLOW);
    result.append("[WARNING] ");
    break;
  case (LOG_ERROR):
    result.append(COLOR_RED);
    result.append("[ERROR] ");
    break;
  }
  std::strftime(timestamp, sizeof(timestamp), "[%y/%m/%d %H:%M:%S] ",
                gmtime(&curr_time));
  result.append(timestamp);
  result.append(message);
  switch (level) {
  case (LOG_ERROR):
  case (LOG_WARNING):
    result.append(COLOR_END);
  case (LOG_INFO): // to silence the warning
    break;
  }
  std::cout << result << std::endl;
  return;
}
