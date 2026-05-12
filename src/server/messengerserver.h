#pragma once

#include <memory>
#include <thread>
#include <vector>

#include "connectionacceptor.h"


namespace will {


struct ClientHub;
class ListenSocketStopSignals;


class MessengerServer {
public:
	MessengerServer();
	~MessengerServer();

	void run();

private:
	void serve_clients(const ListenSocketStopSignals& stop_signals);

	ConnectionAcceptor acceptor_;
	std::unique_ptr<ClientHub> hub_;
	std::vector<std::jthread> client_threads_;
};


} // namespace will
