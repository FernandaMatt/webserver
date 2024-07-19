#include "Logger.hpp"
#include <ctime>

Logger::Logger() {}
Logger::~Logger() {}

void Logger::log(LOG_LEVEL level, const std::string &message) throw() {
		if (LOGGING_ENABLED) {
			const std::time_t	curr_time = std::time(0);
			std::string				result;
			char							timestamp[21] = {0};

			std::ostream &output = (level == LOG_INFO) ? std::cout : std::cerr;

			switch (level) {
				case (LOG_INFO):
					result.append(BLUE);
					result.append("[INFO]\t  ");
					break;
				case (LOG_WARNING):
					if (DEBUG_ENABLED)
					{
						result.append(YLW);
						result.append("[WARNING] ");
					}
					break;
				case (LOG_ERROR):
					if (DEBUG_ENABLED)
					{
						result.append(RED);
						result.append("[ERROR]\t  ");
					}
					break;
			}
			std::strftime(timestamp, sizeof(timestamp), "[%y/%m/%d %H:%M:%S] ", localtime(&curr_time));
			result.append(timestamp);
			result.append(message);
			result.append(RST);

			output << result << std::endl;
			return;
		}
}
