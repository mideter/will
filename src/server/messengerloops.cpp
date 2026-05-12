#include "messengerloops.h"

#include <exception>
#include <iostream>

#include "client.h"
#include "clienthub.h"


namespace will {


std::mutex MessengerLoops::frame_log_mutex_{};


void MessengerLoops::gather_recipients_excluding_sender(const ClientHub& hub,
														const Client& sender,
														std::vector<std::shared_ptr<Client>>& recipients_out)
{
	recipients_out.clear();
	recipients_out.reserve(8);

	for (const std::shared_ptr<Client>& c : hub.snapshot()) {
		if (c.get() != &sender)
			recipients_out.push_back(c);
	}
}


void MessengerLoops::log_frame_no_other_peers(const Client& sender, const std::vector<char>& payload)
{
	const auto plen = payload.size();

	std::lock_guard io_lock(frame_log_mutex_);

	std::cout << "Frame from " << sender.address()
			  << " (no other peers; logged only): header payload_len=" << static_cast<unsigned int>(plen)
			  << ", body (" << plen << " byte" << (plen == 1 ? "" : "s") << "): ";

	std::cout.write(payload.data(), static_cast<std::streamsize>(payload.size()));
	std::cout << std::endl;
}


void MessengerLoops::send_payload_to_recipients(ClientHub& hub,
												const std::vector<std::shared_ptr<Client>>& recipients,
												const std::vector<char>& payload)
{
	for (const std::shared_ptr<Client>& peer : recipients) {
		try {
			peer->send_frame(payload);
		}
		catch (const std::exception& e) {
			std::cerr << "Broadcast send failed to " << peer->address() << ": " << e.what() << std::endl;
			hub.remove(peer.get());
		}
	}
}


void MessengerLoops::log_frame_broadcast_summary(const Client& sender,
												 std::size_t recipient_count,
												 const std::vector<char>& payload)
{
	const auto plen = payload.size();

	std::lock_guard io_lock(frame_log_mutex_);

	std::cout << "Broadcast from " << sender.address() << " to " << recipient_count
			  << " peer(s): header payload_len=" << static_cast<unsigned int>(plen) << ", body (" << plen
			  << " byte" << (plen == 1 ? "" : "s") << "): ";

	std::cout.write(payload.data(), static_cast<std::streamsize>(payload.size()));
	std::cout << std::endl;
}


void MessengerLoops::broadcast_from_sender(ClientHub& hub, Client& sender, const std::vector<char>& payload)
{
	std::vector<std::shared_ptr<Client>> recipients;
	gather_recipients_excluding_sender(hub, sender, recipients);

	if (recipients.empty()) {
		log_frame_no_other_peers(sender, payload);
		return;
	}

	send_payload_to_recipients(hub, recipients, payload);
	log_frame_broadcast_summary(sender, recipients.size(), payload);
}


void MessengerLoops::run_client_session(ClientHub& hub, std::shared_ptr<Client> client)
{
	hub.add(client);

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

	hub.remove(client.get());
}


} // namespace will
