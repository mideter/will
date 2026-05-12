#pragma once

#include <thread>
#include <vector>

#include "connectionacceptor.h"
#include "clienthub.h"
#include "listensocketstopsignals.h"


namespace will {


class MessengerServer {
public:
	MessengerServer();
	~MessengerServer();

	void run();

private:
	struct Peers {
		ClientHub clients;
		std::vector<std::jthread> threads;
	};

	ConnectionAcceptor acceptor_;
	ListenSocketStopSignals stop_signals_;
	Peers peers_;
};


} // namespace will
