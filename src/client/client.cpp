#include <iostream>

#include "chatsession.h"
#include "messengerclient.h"
#include "defaultwillserver.h"


int main()
try {
	const ServerAddress server_address = defaultWillServerAddress();
	
	MessengerClient client;
	client.connect(server_address);

	ChatSession chat_session(client);
	chat_session.run();

	return 0;
} 
catch (const std::exception& e) {
	std::cerr << e.what() << '\n';
	return 1;
}

