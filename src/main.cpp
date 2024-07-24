#include "config/Parser.hpp"
#include "http/RequestParser.hpp"
#include "utils/defines.hpp"
#include "core/WebServer.hpp"

int main(int argc, char **argv) {
	if (argc > 2 ){
		Logger::log(LOG_ERROR, "Invalid input");
		Logger::log(LOG_INFO, "Try: ./webserv [configuration file] or ./webserv to run default configuration file.");
		return (-1);
	}

	std::string config_file;
	if (argc == 1)
		config_file = "/home/42/webserv/webserver/src/server_local.conf";
	else
		config_file = argv[1];


	Parser		*parser = NULL;
	WebServer	*webServer = NULL;
	std::srand(std::time(0));
	try {
		parser = new Parser(config_file);
		Logger::log(LOG_INFO, "parse of configuration file completed");
		//parser->print_servers_directives();
		webServer= new WebServer(parser->get_servers());
		webServer->run();
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	delete parser;
	delete webServer;
	return 0;
}
