#pragma once

#include "messengerclient.h"


class ChatSession {
public:
	explicit ChatSession(MessengerClient& client);

	void run();

private:
	bool authorize_on_server();
	void receiveLoop() const;

	MessengerClient& client_;
	std::string username_;
};
