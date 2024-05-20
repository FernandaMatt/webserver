#include "Logger.hpp"
#include "Listener.hpp"

int main() {

    Listener listener("0:0:0:0:0:0:0:1", "8080");
		
		listener.listen();
    return 0;
}