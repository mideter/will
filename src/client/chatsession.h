#pragma once

#include "messengerclient.h"


namespace will {


class ChatSession {
public:
    explicit ChatSession(MessengerClient& client);

    void run();

private:
    void receiveLoop() const;

    MessengerClient& client_;
    std::string username_;
};


} // namespace will
