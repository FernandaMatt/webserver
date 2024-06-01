#include "Parser.hpp"
#include "defines.hpp"

int main(int argc, char **argv) {
	if (argc != 2 ){
		std::cerr << RED << "Error: invalid input" << RST << std::endl;
		std::cerr << GRN << "Try: ./webserv [configuration file]" << RST << std::endl;
		return (-1);
	}

	std::string config_file;
	config_file = argv[1];

	try {
		Parser parser(config_file);
		std::vector<Server> servers = parser.get_servers();
		for (size_t i = 0; i < servers.size(); i++) {
			std::cout << "Server [ " << i + 1 << " ]: ";
			servers[i].listen();
			servers[i].print_all_directives();
		}
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
