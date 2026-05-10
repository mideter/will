#pragma once


namespace will {


class ConnectionAcceptor;
class ListenSocketStopSignals;


class MessengerServer {
public:
	void run() const;

private:
	void serve_clients(ConnectionAcceptor& acceptor, const ListenSocketStopSignals& stop_signals) const;
};


} // namespace will
