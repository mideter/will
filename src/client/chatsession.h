#pragma once

#include "willclient.h"

#include <vector>


namespace will {


/** Interactive chat session: loads history and runs send/receive loops. */
class ChatSession {
public:
    explicit ChatSession(WillClient& client);

    void run();

private:
    template<typename Handler>
    void on_server_payload(const std::vector<char>& payload, Handler& handler) const;

    void receiveLoop() const;
    void loadHistory() const;

    WillClient& client_;
};


} // namespace will
