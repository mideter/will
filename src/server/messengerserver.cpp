#include "messengerserver.h"

#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include "client.h"
#include "messengerloops.h"


namespace will {


MessengerServer::MessengerServer() = default;


MessengerServer::~MessengerServer() = default;


void MessengerServer::run()
{
	peers_.threads.clear();
	peers_.clients.reset();

	while (true) {
		try {
			std::optional<ClientConnection> accepted = acceptor_.accept_next();
			// No connection: accept loop ended on graceful shutdown (e.g. SIGINT/SIGTERM).
			if (!accepted.has_value())
				break;

			peers_.threads.emplace_back(MessengerLoops::run_client_session,
										std::ref(peers_.clients),
										std::make_shared<Client>(std::move(*accepted)));
		}
		catch (const std::exception& e) {
			std::cerr << "Session error: " << e.what() << '\n';
		}
	}
}


} // namespace will
