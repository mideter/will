#pragma once

#include "willclient.h"


namespace will {


class ChatSession {
public:
    explicit ChatSession(WillClient& client);

    void run();

private:
    void receiveLoop() const;
    void loadHistory() const;

    WillClient& client_;
};


} // namespace will
