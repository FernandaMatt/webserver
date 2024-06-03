#ifndef LOGGER_HPP
# define LOGGER_HPP

# include <iostream>
# include "defines.hpp"

enum LOG_LEVEL { LOG_INFO, LOG_ERROR, LOG_WARNING };

class Logger {
public:
  ~Logger(void);
  static void log(LOG_LEVEL, const std::string &message) throw();

private:
  Logger(void);
};

#endif
