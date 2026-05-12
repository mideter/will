#pragma once

#include <memory>
#include <thread>
#include <vector>


namespace will {


struct ClientHub;
class ConnectionAcceptor;
class ListenSocketStopSignals;


class MessengerServer {
public:
	MessengerServer();
	~MessengerServer();

	void run();

private:
	void serve_clients(ConnectionAcceptor& acceptor, const ListenSocketStopSignals& stop_signals);

	std::unique_ptr<ClientHub> hub_;
	std::vector<std::jthread> client_threads_;
};


} // namespace will
