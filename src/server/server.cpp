#include <iostream>

#include "messengerserver.h"
#include "port.h"


int main()
try {
	constexpr const char* version = "2.0.0";
	std::cout << "Starting Will Messenger Server v" << version << '\n';

	const MessengerServer server;

	server.run();

	return 0;
}
catch (const std::exception& e) {
	std::cerr << "Server error: " << e.what() << '\n';
	return 1;
}
