#include <iostream>
#include <string>
#include <thread>

#include "messengerclient.h"
#include "ipv4.h"
#include "port.h"
#include "serveraddress.h"


class SingaporeServer {
public:
	inline static const Port port{8080};
	inline static const IPv4 ip{"139.59.117.130"};
};


class LocalServer {
public:
	inline static const Port port{8080};
	inline static const IPv4 ip{"127.0.0.1"};
};


int main()
try {
	const ServerAddress server_address(SingaporeServer::ip, SingaporeServer::port);
	
	MessengerClient client;
	client.connect(server_address);

	std::cout << "Connected to Will chat. Type messages and press Enter.\n";
	std::cout << "Press Ctrl+D to exit.\n";

	std::thread receiver([&client]() {
		try {
			while (true) {
				const std::string incoming = client.receive();
				if (incoming.empty()) {
					std::cout << "\nDisconnected from chat.\n";
					break;
				}
				std::cout << "Peer: " << incoming;
				if (!incoming.empty() && incoming.back() != '\n')
					std::cout << '\n';
				std::cout.flush();
			}
		}
		catch (const std::exception& e) {
			std::cerr << "\nReceive error: " << e.what() << '\n';
		}
	});

	std::string line;
	while (std::getline(std::cin, line))
		client.send(line + '\n');

	client.shutdown();
	receiver.join();

	return 0;
} 
catch (const std::exception& e) {
	std::cerr << e.what() << '\n';
	return 1;
}

