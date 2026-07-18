#pragma once

#include "wiremessage.h"


namespace will {


class ConsoleUi;
class WillClient;


class LoadingHistoryMessageHandler final {
public:
    explicit LoadingHistoryMessageHandler(ConsoleUi& ui);

    bool history_finished() const noexcept { return history_finished_; }

    void on(const ServerMessage& message);

private:
    ConsoleUi& ui_;
    bool history_finished_ = false;
};


class ReceivingMessageHandler final {
public:
    ReceivingMessageHandler(const WillClient& client, ConsoleUi& ui);

    void on(const ServerMessage& message);

private:
    const WillClient& client_;
    ConsoleUi& ui_;
};


} // namespace will
