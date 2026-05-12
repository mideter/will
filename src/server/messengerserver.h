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
	ConnectionAcceptor acceptor_;
	ListenSocketStopSignals stop_signals_;
	std::unique_ptr<ClientHub> hub_;
	std::vector<std::jthread> client_threads_;
};


} // namespace will
