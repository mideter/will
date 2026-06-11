#pragma once

#include "wiremessage.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"


namespace will {


class WillClient;


class ChatUiVisitor final : public ServerMessageVisitor {
public:
    enum class Context { LoadingHistory, Receiving };

    explicit ChatUiVisitor(Context context, const WillClient* client = nullptr);

    bool history_finished() const noexcept { return history_finished_; }

    void on(const LoginResponseMessage& message) override;
    void on(const AuthRequiredMessage& message) override;
    void on(const ServerReceiptAckMessage& message) override;
    void on(const HistoryItemMessage& message) override;
    void on(const HistoryEndMessage& message) override;
    void on(const UserChatMessage& message) override;

private:
    void throw_unexpected_history() const;

    Context context_;
    const WillClient* client_;
    bool history_finished_ = false;
};


} // namespace will
