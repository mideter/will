#pragma once

#include "consoleui.h"
#include "willclient.h"

#include "proto/messenger.pb.h"


namespace will {


/** Interactive chat session: loads history, then stdin send loop with inbound/closed handlers. */
class ChatSession {
public:
    ChatSession(WillClient& client, ConsoleUi& ui);

    void run();

private:
    template<typename Handler>
    void on_server_event(const v1::ServerEvent& event, Handler& handler) const;

    void loadHistory() const;

    WillClient& client_;
    ConsoleUi& ui_;
};


} // namespace will
