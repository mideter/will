#include "messengerserver.h"


namespace will {


MessengerServer::MessengerServer(ServerConfig config)
    : server_(std::move(config))
{}


void MessengerServer::run()
{
    server_.run();
}


} // namespace will
