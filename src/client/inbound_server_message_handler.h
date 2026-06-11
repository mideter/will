#pragma once

#include "wiremessage.h"


namespace will {


class WillClient;


class LoadingHistoryMessageHandler final {
public:
    bool history_finished() const noexcept { return history_finished_; }

    void on(const ServerMessage& message);

private:
    bool history_finished_ = false;
};


class ReceivingMessageHandler final {
public:
    explicit ReceivingMessageHandler(const WillClient& client);

    void on(const ServerMessage& message);

private:
    const WillClient& client_;
};


} // namespace will
