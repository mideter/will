#include "messengerserver.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <vector>

#include "defaultwillserver.h"
#include "listensocketstopsignals.h"
#include "serveraddress.h"
#include "socketerror.h"
#include "sockethandle.h"
#include "willprotocol.h"


namespace will {


namespace {

	
/** Exactly {@code len} bytes unless peer closes before passing the first byte of this chunk (then {@code false}). Truncated after any byte ⇒ throws. */
bool recv_exact_relaxed_eof_before_first_byte(const ClientConnection& from,
											  char* data,
											  std::size_t len)
{
	std::size_t got = 0;
	while (got < len) {
		std::size_t chunk = 0;
		if (!from.recv_some(data + got, len - got, chunk)) {
			if (got != 0)
				throw std::runtime_error{"Will relay: connection closed mid-frame"};
			return false;
		}
		got += chunk;
	}
	return true;
}


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


std::optional<ClientConnection> accept_client_or_stop(const SocketHandle& server_socket,
													   const ListenSocketStopSignals& stop_signals)
{
	try {
		return ClientConnection::accept_from(server_socket, stop_signals);
	}
	catch (const std::system_error&) {
		if (stop_signals.shutdown_requested())
			return std::nullopt;
		throw;
	}
}


} // namespace


SocketHandle MessengerServer::create_listen_socket() const
{
	SocketHandle server_socket(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));

	int opt = 1;
	if (::setsockopt(server_socket.get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw SocketError("setsockopt failed");

	return server_socket;
}


void MessengerServer::bind_and_listen(const SocketHandle& server_socket) const
{
	ServerAddress server = ServerAddress::any(defaultWillServerAddress().port_);

	if (::bind(server_socket.get(), reinterpret_cast<sockaddr*>(&server.address_), sizeof(server.address_)) < 0)
		throw SocketError("bind failed");

	if (::listen(server_socket.get(), Backlog) < 0)
		throw SocketError("listen failed");
}


void MessengerServer::serve_clients(const SocketHandle& server_socket,
									const ListenSocketStopSignals& stop_signals) const
{
	while (true) {
		try {
			if (stop_signals.shutdown_requested())
				break;

			std::cout << "Waiting for first client..." << std::endl;
			std::optional<ClientConnection> first_opt = accept_client_or_stop(server_socket, stop_signals);
			if (!first_opt.has_value())
				break;

			ClientConnection first_client = std::move(*first_opt);
			log_client_connected(first_client);

			ChatPeerFdRegistration peer_fds_raii{};
			ChatPeerFdRegistration::assign(first_client.socket_fd(), -1);

			std::cout << "Waiting for second client..." << std::endl;
			std::optional<ClientConnection> second_opt = accept_client_or_stop(server_socket, stop_signals);
			if (!second_opt.has_value()) {
				first_client.shutdown();
				break;
			}

			ClientConnection second_client = std::move(*second_opt);
			log_client_connected(second_client);

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


void MessengerServer::log_client_connected(const ClientConnection& client)
{
	std::cout << "Client " << client.address() << " connected" << std::endl;
}


void MessengerServer::run() const
{
	SocketHandle server_socket = create_listen_socket();
	bind_and_listen(server_socket);

	const ListenSocketStopSignals stop_signals{server_socket.get()};
	serve_clients(server_socket, stop_signals);
}


void MessengerServer::run_chat_session(const ClientConnection& first, const ClientConnection& second)
{
	std::atomic<bool> stopping{false};

	auto stop_session = [&]() {
		if (stopping.exchange(true))
			return;

		first.shutdown();
		second.shutdown();
	};

	auto relay_with_stop = [&](const ClientConnection& from, const ClientConnection& to) {
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


void MessengerServer::relay_messages(const ClientConnection& from, const ClientConnection& to)
{
	static std::mutex relay_log_mutex;

	while (true) {
		char header_buf[4];
		if (!recv_exact_relaxed_eof_before_first_byte(from, header_buf, sizeof(header_buf)))
			return;

		const unsigned char* const header_u = reinterpret_cast<unsigned char*>(header_buf);
		const std::uint32_t len_u32 = TcpFrame::read_u32_be(header_u);
		const auto plen = static_cast<std::size_t>(len_u32);
		if (plen > TcpFrame::max_payload_bytes)
			throw std::runtime_error{"Will relay: frame exceeds TcpFrame::max_payload_bytes"};

		std::vector<char> payload(plen);
		if (!payload.empty()) {
			if (!recv_exact_relaxed_eof_before_first_byte(from, payload.data(), payload.size()))
				throw std::runtime_error{"Will relay: connection closed mid-frame"};
		}

		{
			std::lock_guard<std::mutex> lock(relay_log_mutex);
			std::cout << "Frame " << from.address() << " -> " << to.address() << ": header payload_len="
					  << len_u32 << ", body (" << plen << " byte" << (plen == 1 ? "" : "s") << "): ";
			std::cout.write(payload.data(), static_cast<std::streamsize>(payload.size()));
			std::cout << std::endl;
		}

		to.send_all(header_buf, sizeof(header_buf));
		if (!payload.empty())
			to.send_all(payload.data(), payload.size());
	}
}


} // namespace will
