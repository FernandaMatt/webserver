#include "Parser.hpp"

int main(int argc, char **argv) {
    std::string config_file;

    if (argc == 1) config_file = "server.conf";
    else if (argc == 2) config_file = argv[1];
    else std::cerr << "Error: invalid input" << std::endl;

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