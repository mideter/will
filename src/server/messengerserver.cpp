#include "messengerserver.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include <array>
#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <system_error>
#include <thread>

#include "defaultwillserver.h"
#include "listensocketstopsignals.h"
#include "serveraddress.h"
#include "socketerror.h"
#include "sockethandle.h"


namespace {

std::optional<ClientConnection> accept_client_or_stop(const SocketHandle& server_socket,
													   const ListenSocketStopSignals& stop_signals)
{
	try {
		return ClientConnection::accept_from(server_socket);
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

			std::cout << "Waiting for second client..." << std::endl;
			std::optional<ClientConnection> second_opt = accept_client_or_stop(server_socket, stop_signals);
			if (!second_opt.has_value()) {
				first_client.shutdown();
				break;
			}

			ClientConnection second_client = std::move(*second_opt);
			log_client_connected(second_client);

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
	std::array<char, BufferSize> buffer{};
	std::size_t received = 0;

	while (from.recv_some(buffer.data(), buffer.size(), received)) {
		{
			std::lock_guard<std::mutex> lock(relay_log_mutex);
			std::cout << "Message " << from.address() << " -> " << to.address()
					  << " (" << received << " byte" << (received == 1 ? "" : "s") << "): ";
			std::cout.write(buffer.data(), static_cast<std::streamsize>(received));
			std::cout << std::endl;
		}
		to.send_all(buffer.data(), received);
	}
}
