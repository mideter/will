#pragma once

#include "messengerclient.h"


class ChatSession {
public:
	explicit ChatSession(MessengerClient& client);

	void run() const;

private:
	void receiveLoop() const;

	MessengerClient& client_;
};
