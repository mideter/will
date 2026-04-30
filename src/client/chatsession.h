#pragma once

#include "messengerclient.h"


class ChatSession {
public:
	explicit ChatSession(MessengerClient& client);

	void run() const;

private:
	bool authorize_on_server() const;
	void receiveLoop() const;

	MessengerClient& client_;
};
