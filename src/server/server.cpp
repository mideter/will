#include <iostream>

#include "willserver.h"
#include "serverconfigparser.h"


int main(int argc, char* argv[])
try {
    const will::ServerConfig config = will::ServerConfigParser::parse(argc, argv);

    will::WillServer server(config);
    server.run();

    return 0;
}
catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << '\n';
    return 1;
}
