#include "messengerserver.h"

#include <atomic>
#include <iostream>
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


/** SIGTERM handler shuts these down so recv/send unblock during graceful stop */
struct ChatPeerFdRegistration {
	~ChatPeerFdRegistration()
	{
		ListenSocketStopSignals::set_chat_peer_fds(-1, -1);
	}

	static void assign(int peer_a_fd, int peer_b_fd)
	{
		ListenSocketStopSignals::set_chat_peer_fds(peer_a_fd, peer_b_fd);
	}
};


} // namespace


void MessengerServer::serve_clients(ConnectionAcceptor& acceptor,
									  const ListenSocketStopSignals& stop_signals) const
{
	while (true) {
		try {
			if (stop_signals.shutdown_requested())
				break;

			std::cout << "Waiting for first client..." << std::endl;
			std::optional<ClientConnection> first_opt = acceptor.accept_next(stop_signals);
			if (!first_opt.has_value())
				break;

			Client first_client(std::move(*first_opt));
			std::cout << "Client " << first_client.address() << " connected" << std::endl;

			ChatPeerFdRegistration peer_fds_raii{};
			ChatPeerFdRegistration::assign(first_client.socket_fd(), -1);

			std::cout << "Waiting for second client..." << std::endl;
			std::optional<ClientConnection> second_opt = acceptor.accept_next(stop_signals);
			if (!second_opt.has_value()) {
				first_client.shutdown();
				break;
			}

			Client second_client(std::move(*second_opt));
			std::cout << "Client " << second_client.address() << " connected" << std::endl;

			ChatPeerFdRegistration::assign(first_client.socket_fd(), second_client.socket_fd());

			std::cout << "Chat session started" << std::endl;
			run_chat_session(first_client, second_client);
			std::cout << "Chat session ended" << std::endl;
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


void MessengerServer::run_chat_session(const Client& first, const Client& second)
{
	std::atomic<bool> stopping{false};

	auto stop_session = [&]() {
		if (stopping.exchange(true))
			return;

		first.shutdown();
		second.shutdown();
	};

	auto relay_with_stop = [&](const Client& from, const Client& to) {
		try {
			relay_messages(from, to);
		}
		catch (const std::exception& e) {
			std::cerr << "Relay error: " << e.what() << '\n';
		}
		stop_session();
	};

	std::thread first_to_second(relay_with_stop, std::cref(first), std::cref(second));
	std::thread second_to_first(relay_with_stop, std::cref(second), std::cref(first));

	first_to_second.join();
	second_to_first.join();
}


void MessengerServer::relay_messages(const Client& from, const Client& to)
{
	static std::mutex relay_log_mutex;

	while (true) {
		std::vector<char> payload;
		if (!from.recv_frame(payload))
			return;

		const std::size_t plen = payload.size();
		const auto len_u32 = static_cast<unsigned int>(plen);

		{
			std::lock_guard<std::mutex> lock(relay_log_mutex);
			std::cout << "Frame " << from.address() << " -> " << to.address() << ": header payload_len="
					  << len_u32 << ", body (" << plen << " byte" << (plen == 1 ? "" : "s") << "): ";
			std::cout.write(payload.data(), static_cast<std::streamsize>(payload.size()));
			std::cout << std::endl;
		}

		to.send_frame(payload);
	}
}


} // namespace will
