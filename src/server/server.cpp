#include <iostream>

#include "willserver.h"
#include "serverconfigparser.h"


int main(int argc, char* argv[])
try {
    const will::cli::ServerConfigParser cli(argc, argv);

    will::WillServer server(cli.server_config());
    server.run();

    return 0;
}
catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << '\n';
    return 1;
}
