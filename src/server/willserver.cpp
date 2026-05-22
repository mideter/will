#include "willserver.h"
#include "asioserver.h"

#include <iostream>


namespace will {


WillServer::WillServer(ServerConfig config)
    : AsioMessengerServer(std::move(config))
{}


void WillServer::run()
{
    log_startup(config());
    AsioMessengerServer::run();
}


void WillServer::log_startup(const ServerConfig& config)
{
    std::cout << "Starting Will Messenger Server v" << Version << " on port " << config.listen_port()
              << " (max " << config.max_connections() << " clients, "
              << config.io_threads() << " io threads)\n";
}


} // namespace will
