#include "chatsession.h"

#include <atomic>
#include <iostream>
#include <string>
#include <thread>


ChatSession::ChatSession(MessengerClient& client)
	: client_(client)
{}


void ChatSession::run() const
{
	std::cout << "Connected to Will chat. Type messages and press Enter.\n";
	std::cout << "Press Ctrl+D to exit.\n";

	std::atomic<bool> receive_finished{false};
	std::jthread receiver([this, &receive_finished]() {
		receiveLoop();
		receive_finished.store(true);
	});

	std::string line;
	while (!receive_finished.load() && std::getline(std::cin, line))
		client_.send(line + '\n');

	client_.shutdown();
}


void ChatSession::receiveLoop() const
{
	try {
		while (true) {
			const std::string incoming = client_.receive();

			if (incoming.empty()) {
				std::cout << std::endl << "Disconnected from chat." << std::endl;
				break;
			}

			std::cout << "Peer: " << incoming;
			
			if (incoming.back() != '\n') 
				std::cout << std::endl;

			std::cout.flush();
		}
	}
	catch (const std::exception& e) {
		std::cerr << std::endl << "Receive error: " << e.what() << std::endl;
	}
}
