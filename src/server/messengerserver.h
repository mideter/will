#pragma once

#include <memory>


namespace will {


struct ClientHub;
class ConnectionAcceptor;
class ListenSocketStopSignals;


class MessengerServer {
public:
	MessengerServer() = default;
	~MessengerServer();

	void run();

private:
	void serve_clients(ConnectionAcceptor& acceptor, const ListenSocketStopSignals& stop_signals);

	std::shared_ptr<ClientHub> hub_;
};


} // namespace will
