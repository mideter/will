#include "chatsession.h"

#include <atomic>
#include <iostream>
#include <string>
#include <thread>


ChatSession::ChatSession(MessengerClient& client)
	: client_(client)
{}


bool ChatSession::authorize_on_server()
{
	std::string username;
	std::string password;

	std::cout << "Login: ";
	if (!std::getline(std::cin, username))
		return false;

	std::cout << "Password: ";
	if (!std::getline(std::cin, password))
		return false;

	client_.send("AUTH " + username + " " + password + "\n");
	const std::string response = client_.receive();

	if (response != "AUTH_OK\n") {
		std::cerr << "Authorization failed on server.\n";
		return false;
	}

	username_ = username;
	std::cout << "Authorization successful.\n";
	return true;
}


void ChatSession::run()
{
	if (!authorize_on_server()) {
		client_.shutdown();
		return;
	}

	std::cout << "Connected to Will chat. Type messages and press Enter.\n";
	std::cout << "Press Ctrl+D to exit.\n";

	std::atomic<bool> receive_finished{false};
	std::jthread receiver([this, &receive_finished]() {
		receiveLoop();
		receive_finished.store(true);
	});

	std::string line;
	while (!receive_finished.load() && std::getline(std::cin, line))
		client_.send(username_ + ": " + line + '\n');

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

			std::cout << incoming;
			
			if (incoming.back() != '\n') 
				std::cout << std::endl;

			std::cout.flush();
		}
	}
	catch (const std::exception& e) {
		std::cerr << std::endl << "Receive error: " << e.what() << std::endl;
	}
}
