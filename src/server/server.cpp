#include <iostream>

#include "messengerserver.h"


int main()
try {
	constexpr const char* version = "2.0.2";
	std::cout << "Starting Will Messenger Server v" << version << '\n';

	const will::MessengerServer server;

	server.run();

	return 0;
}
catch (const std::exception& e) {
	std::cerr << "Server error: " << e.what() << '\n';
	return 1;
}
