#include "willserver.h"
#include "serverconfigvalidator.h"

#include <iostream>


namespace will {


WillServer::WillServer(ServerConfig config)
    : config_(ServerConfigValidator::accept(std::move(config)))
    , persistence_(config_.db_path)
    , server_(config_, persistence_.ports())
{}


void WillServer::run()
{
    log_startup(config_);
    server_.run();
}


void WillServer::log_startup(const ServerConfig& config)
{
    std::cout << "Starting Will Messenger Server v" << Version << " on port " << config.listen_port
              << " (max " << config.max_connections << " clients, gRPC)\n";
}


} // namespace will
