#include "messengerserver.h"

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include "client.h"
#include "clientconnection.h"
#include "connectionacceptor.h"


namespace will {


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


namespace {


std::mutex frame_log_mutex;


void broadcast_from_sender(ClientHub& hub, Client& sender, const std::vector<char>& payload)
{
	std::vector<std::shared_ptr<Client>> recipients;
	recipients.reserve(8);
	const std::vector<std::shared_ptr<Client>> now = hub.snapshot();
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
			peer->shutdown();
			hub.remove(peer.get());
		}
	}

	std::lock_guard io_lock(frame_log_mutex);
	std::cout << "Broadcast from " << sender.address() << " to " << recipients.size()
			  << " peer(s): header payload_len=" << static_cast<unsigned int>(plen) << ", body (" << plen
			  << " byte" << (plen == 1 ? "" : "s") << "): ";
	std::cout.write(payload.data(), static_cast<std::streamsize>(payload.size()));
	std::cout << std::endl;
}


void reader_main(ClientHub& hub, std::shared_ptr<Client> client, int sig_slot)
{
	hub.add(client);

	std::cout << "Client " << client->address() << " connected" << std::endl;

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

	client->shutdown();

	if (sig_slot >= 0)
		ListenSocketStopSignals::unregister_chat_peer_fd(sig_slot);
	hub.remove(client.get());
}


} // namespace


MessengerServer::MessengerServer()
	: stop_signals_{acceptor_.listen_fd()}
{}


MessengerServer::~MessengerServer() = default;


void MessengerServer::run()
{
	client_threads_.clear();
	hub_ = std::make_unique<ClientHub>();

	while (true) {
		try {
			std::optional<AcceptedConnection> accepted = acceptor_.accept_next(stop_signals_);
			if (!accepted.has_value())
				break;

			AcceptedConnection ac = std::move(*accepted);
			client_threads_.emplace_back(reader_main, std::ref(*hub_), std::make_shared<Client>(std::move(ac.connection)),
										 ac.sig_slot);
		}
		catch (const std::exception& e) {
			std::cerr << "Session error: " << e.what() << '\n';
		}
	}
}


} // namespace will
