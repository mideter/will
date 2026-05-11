#include "messengerserver.h"

#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include "client.h"
#include "clientconnection.h"
#include "connectionacceptor.h"
#include "listensocketstopsignals.h"


namespace will {


namespace {


struct ClientHub {
	mutable std::mutex mutex_;
	std::vector<std::shared_ptr<Client>> clients_;

	void add(std::shared_ptr<Client> client)
	{
		std::lock_guard lock(mutex_);
		clients_.push_back(std::move(client));
	}

	void remove(const Client* identity)
	{
		std::lock_guard lock(mutex_);
		std::erase_if(clients_, [identity](const std::shared_ptr<Client>& c) {
			return c.get() == identity;
		});
	}

	std::vector<std::shared_ptr<Client>> snapshot() const
	{
		std::lock_guard lock(mutex_);
		return clients_;
	}
};


std::mutex frame_log_mutex;


void broadcast_from_sender(const std::shared_ptr<ClientHub>& hub, Client& sender,
						   const std::vector<char>& payload)
{
	std::vector<std::shared_ptr<Client>> recipients;
	recipients.reserve(8);
	const std::vector<std::shared_ptr<Client>> now = hub->snapshot();
	for (const std::shared_ptr<Client>& c : now) {
		if (c.get() != &sender)
			recipients.push_back(c);
	}

	const auto plen = payload.size();

	if (recipients.empty()) {
		std::lock_guard io_lock(frame_log_mutex);
		std::cout << "Frame from " << sender.address()
				  << " (no other peers; logged only): header payload_len=" << static_cast<unsigned int>(plen)
				  << ", body (" << plen << " byte" << (plen == 1 ? "" : "s") << "): ";
		std::cout.write(payload.data(), static_cast<std::streamsize>(payload.size()));
		std::cout << std::endl;
		return;
	}

	for (const std::shared_ptr<Client>& peer : recipients) {
		try {
			peer->send_frame(payload);
		}
		catch (const std::exception& e) {
			std::cerr << "Broadcast send failed to " << peer->address() << ": " << e.what() << '\n';
			try {
				peer->shutdown();
			}
			catch (const std::exception&) {
			}
			hub->remove(peer.get());
		}
	}

	std::lock_guard io_lock(frame_log_mutex);
	std::cout << "Broadcast from " << sender.address() << " to " << recipients.size()
			  << " peer(s): header payload_len=" << static_cast<unsigned int>(plen) << ", body (" << plen
			  << " byte" << (plen == 1 ? "" : "s") << "): ";
	std::cout.write(payload.data(), static_cast<std::streamsize>(payload.size()));
	std::cout << std::endl;
}


void reader_main(std::shared_ptr<ClientHub> hub, std::shared_ptr<Client> client, int sig_slot)
{
	hub->add(client);

	try {
		while (true) {
			std::vector<char> payload;
			if (!client->recv_frame(payload))
				break;

			broadcast_from_sender(hub, *client, payload);
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Reader error " << client->address() << ": " << e.what() << '\n';
	}

	try {
		client->shutdown();
	}
	catch (const std::exception&) {
	}

	if (sig_slot >= 0)
		ListenSocketStopSignals::unregister_chat_peer_fd(sig_slot);
	hub->remove(client.get());
}


} // namespace


void MessengerServer::serve_clients(ConnectionAcceptor& acceptor,
									  const ListenSocketStopSignals& stop_signals) const
{
	const std::shared_ptr<ClientHub> hub = std::make_shared<ClientHub>();

	while (true) {
		try {
			std::optional<AcceptedConnection> accepted = acceptor.accept_next(stop_signals);
			if (!accepted.has_value())
				break;

			const int sig_slot = accepted->sig_slot;
			auto client = std::make_shared<Client>(std::move(accepted->connection));
			std::cout << "Client " << client->address() << " connected" << std::endl;

			std::thread([hub, client, sig_slot]() {
				reader_main(hub, std::move(client), sig_slot);
			}).detach();
		}
		catch (const std::exception& e) {
			std::cerr << "Session error: " << e.what() << '\n';
		}
	}
}


void MessengerServer::run() const
{
	ConnectionAcceptor acceptor;
	const ListenSocketStopSignals stop_signals{acceptor.listen_fd()};
	serve_clients(acceptor, stop_signals);
}


} // namespace will
