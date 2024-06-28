#include "Parser.hpp"
#include "src/RequestParser.hpp"
#include "defines.hpp"
#include "WebServer.hpp"

int main(int argc, char **argv) {
	if (argc != 2 ){
		Logger::log(LOG_ERROR, "Invalid input");
		Logger::log(LOG_INFO, "Try: ./webserv [configuration file]");
		return (-1);
	}

	std::string config_file = argv[1];
	Parser		*parser = NULL;
	WebServer	*webServer = NULL;

	try {
		parser = new Parser(config_file);
		Logger::log(LOG_INFO, "parse of configuration file completed");
		// parser->print_servers_directives();
		webServer= new WebServer(parser->get_servers());
		webServer->run();
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	// try{
	// 	RequestParser reqParser;

	// 	httpRequest req = reqParser.parseRequest("DELETE /home HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/7.68.0\r\nAccept: */*\r\n\r\n");
	// 	req.printRequest();
	// }
	// catch (std::exception &e) {
	// 	std::cerr << e.what() << std::endl;
	// }

	delete parser;
	delete webServer;
	return 0;
}
