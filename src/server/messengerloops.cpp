#include "messengerloops.h"

#include <exception>
#include <iostream>

#include "client.h"
#include "clienthub.h"
#include "listensocketstopsignals.h"


namespace will {


std::mutex MessengerLoops::frame_log_mutex_{};


void MessengerLoops::broadcast_from_sender(ClientHub& hub, Client& sender, const std::vector<char>& payload)
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
		std::lock_guard io_lock(frame_log_mutex_);
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

	std::lock_guard io_lock(frame_log_mutex_);
	std::cout << "Broadcast from " << sender.address() << " to " << recipients.size()
			  << " peer(s): header payload_len=" << static_cast<unsigned int>(plen) << ", body (" << plen
			  << " byte" << (plen == 1 ? "" : "s") << "): ";
	std::cout.write(payload.data(), static_cast<std::streamsize>(payload.size()));
	std::cout << std::endl;
}


void MessengerLoops::reader_main(ClientHub& hub, std::shared_ptr<Client> client, int sig_slot)
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


} // namespace will
