#include "messengerserver.h"

#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include "client.h"
#include "messengerloops.h"


namespace will {


MessengerServer::MessengerServer()
	: stop_signals_{acceptor_.listen_fd()}
{}


MessengerServer::~MessengerServer() = default;


void MessengerServer::run()
{
	peers_.threads.clear();
	peers_.clients.reset();

	while (true) {
		try {
			std::optional<AcceptedConnection> accepted = acceptor_.accept_next(stop_signals_);
			if (!accepted.has_value())
				break;

			AcceptedConnection ac = std::move(*accepted);
			
			peers_.threads.emplace_back(MessengerLoops::reader_main, 
										std::ref(peers_.clients), 
										std::make_shared<Client>(std::move(ac.connection)),
										ac.sig_slot);
		}
		catch (const std::exception& e) {
			std::cerr << "Session error: " << e.what() << '\n';
		}
	}
}


} // namespace will
