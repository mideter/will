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
    static void printHistoryItem(const HistoryItemPayload& item);

    WillClient& client_;
};


} // namespace will
