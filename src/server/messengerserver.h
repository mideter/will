#pragma once


namespace will {


class Client;
class ConnectionAcceptor;
class ListenSocketStopSignals;


class MessengerServer {
public:
	void run() const;

private:
	void serve_clients(ConnectionAcceptor& acceptor, const ListenSocketStopSignals& stop_signals) const;

	static void run_chat_session(const Client& first, const Client& second);
	static void relay_messages(const Client& from, const Client& to);
};


} // namespace will
