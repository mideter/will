#pragma once

#include "grpcserver.h"
#include "serverconfig.h"
#include "sqlite_persistence_bundle.h"


namespace will {


class WillServer {
public:
	static constexpr const char* Version = "6.0.0";

	explicit WillServer(ServerConfig config = {});

	void run();

private:
	static void log_startup(const ServerConfig& config);

	ServerConfig config_;
	SqlitePersistenceBundle persistence_;
	GrpcMessengerServer server_;
};


} // namespace will
