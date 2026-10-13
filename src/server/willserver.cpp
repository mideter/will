#include "willserver.h"
#include "serverconfigvalidator.h"

#include <iostream>


namespace will {


WillServer::WillServer(ServerConfig config)
	: config_(ServerConfigValidator::accept(std::move(config)))
	, persistence_(config_.db_path)
	, server_(config_, persistence_.world())
{}


void WillServer::run()
{
	log_startup(config_);
	// What is kept but does not agree is passed over, not lost: it is told, to be looked into.
	for (const std::string& what : persistence_.world().unrecollected())
		std::cerr << "Not recollected: " << what << '\n';
	server_.run();
}


void WillServer::log_startup(const ServerConfig& config)
{
	std::cout << "Starting Will Messenger Server v" << Version << " on";
	if (config.plain)
		std::cout << " port " << config.listen_port;
	if (config.plain && config.tls())
		std::cout << " and";
	if (config.tls())
		std::cout << " TLS port " << config.tls_port;
	std::cout << " (max " << config.max_connections << " clients, gRPC)\n";
}


} // namespace will
