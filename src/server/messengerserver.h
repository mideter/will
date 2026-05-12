#pragma once

#include <memory>
#include <thread>
#include <vector>

#include "connectionacceptor.h"
#include "listensocketstopsignals.h"


namespace will {


struct ClientHub;


class MessengerServer {
public:
	MessengerServer();
	~MessengerServer();

	void run();

private:
	struct Peers {
		std::unique_ptr<ClientHub> clients;
		std::vector<std::jthread> threads;
	};

	ConnectionAcceptor acceptor_;
	ListenSocketStopSignals stop_signals_;
	Peers peers_;
};


} // namespace will
