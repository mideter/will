#include <iostream>

#include "messengerserver.h"
#include "serverconfigparser.h"


int main(int argc, char* argv[])
try {
    constexpr const char* version = "4.0.0";
    const will::ServerConfig config = will::ServerConfigParser::parse(argc, argv);

    std::cout << "Starting Will Messenger Server v" << version << " on port " << config.listen_port
              << " (max " << config.max_connections << " clients, " << config.io_threads
              << " io threads)\n";

    will::MessengerServer server(config);
    server.run();

    return 0;
}
catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << '\n';
    return 1;
}
