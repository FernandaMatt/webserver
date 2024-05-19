#ifndef LOGGER_HPP
#define LOGGER_HPP
#pragma once
#include <string>

#ifdef __MACH__
#define COLOR_RED "\033[1;31m"
#define COLOR_YELLOW "\033[1;32m"
#define COLOR_END "\033[0m"
#else
#define COLOR_RED "\u001b[31m"
#define COLOR_YELLOW "\u001b[33m"
#define COLOR_END "\u001b[0m"
#endif

enum LOG_LEVEL { LOG_INFO, LOG_ERROR, LOG_WARNING };

class Logger {
public:
  ~Logger(void);
  static void log(LOG_LEVEL, const std::string) throw();

private:
  Logger(void);
};

#endif // ! LOGGER_HPP
