#include <iostream>

#include "chatsession.h"
#include "messengerclient.h"
#include "ipv4.h"
#include "port.h"
#include "serveraddress.h"


class SingaporeServer {
public:
	inline static const Port port{8080};
	inline static const IPv4 ip{"139.59.117.130"};
};


class RussianServer {
public:
	inline static const Port port{8080};
	inline static const IPv4 ip{"83.217.202.145"};
};


class LocalServer {
public:
	inline static const Port port{8080};
	inline static const IPv4 ip{"127.0.0.1"};
};


int main()
try {
	const ServerAddress server_address(RussianServer::ip, RussianServer::port);
	
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

