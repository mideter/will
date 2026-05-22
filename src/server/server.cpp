#include <iostream>

#include "willserver.h"
#include "serverconfigparser.h"


int main(int argc, char* argv[])
try {
    const will::ServerConfigParser config_parser(argc, argv);

    will::WillServer server(config_parser.config());
    server.run();

    return 0;
}
catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << '\n';
    return 1;
}
