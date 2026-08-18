module;

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

module will.server.protocol_logic;

import will.domain.authenticate_device;
import will.domain.fetch_chat_history;
import will.domain.message_repository;
import will.domain.participant_notifier;
import will.domain.send_chat_message;
import will.domain.timestamp;
import will.domain.user_repository;
import will.server.connectionaccountstore;


namespace will {


namespace {


class SessionParticipantNotifierImpl final : public domain::ParticipantNotifier {
public:
    SessionParticipantNotifierImpl(void* registry, BroadcastChat broadcast)
        : registry_(registry)
        , broadcast_(broadcast)
    {}

    void notify_chat_message(const domain::ChatId chat, const domain::Message& msg,
                             const domain::ParticipantId except_participant) override
    {
        (void)chat;
        broadcast_(registry_, except_participant.value, msg.author_name, msg.body);
    }

    void send_to_participant(const domain::ParticipantId id, const domain::OutboundEvent& ev) override
    {
        (void)id;
        (void)ev;
    }

private:
    void* registry_;
    BroadcastChat broadcast_;
};


} // namespace


struct SessionRuntime::Impl {
    Impl(void* messages, void* users, void* accounts, void* registry, BroadcastChat broadcast)
        : messages_(*static_cast<domain::MessageRepository*>(messages))
        , users_(*static_cast<domain::UserRepository*>(users))
        , accounts_(*static_cast<ConnectionAccountStore*>(accounts))
        , notifier_(std::make_unique<SessionParticipantNotifierImpl>(registry, broadcast))
        , authenticate_device_(users_)
        , send_chat_message_(messages_, *notifier_)
        , fetch_chat_history_(messages_)
    {}

    domain::MessageRepository& messages_;
    domain::UserRepository& users_;
    ConnectionAccountStore& accounts_;
    std::unique_ptr<domain::ParticipantNotifier> notifier_;
    domain::AuthenticateDevice authenticate_device_;
    domain::SendChatMessage send_chat_message_;
    domain::FetchChatHistory fetch_chat_history_;
};


SessionRuntime::SessionRuntime(void* messages, void* users, void* accounts, void* registry,
                               BroadcastChat broadcast)
    : impl_(std::make_unique<Impl>(messages, users, accounts, registry, broadcast))
{}


SessionRuntime::~SessionRuntime() = default;


bool SessionRuntime::has_account(const std::uint64_t session_id) const
{
    return impl_->accounts_.has(session_id);
}


BindTokenResult SessionRuntime::bind_token(const std::uint64_t session_id, const std::string_view token)
{
    const domain::AuthenticateDeviceInput input{token, domain::Timestamp::now()};
    const auto outcome = impl_->authenticate_device_.execute(input);

    if (std::holds_alternative<domain::AuthError>(outcome))
        return {BindTokenResult::Status::InvalidToken, std::nullopt};

    const auto& success = std::get<domain::AuthenticateDeviceSuccess>(outcome);
    BindTokenResult result{BindTokenResult::Status::Ok, impl_->accounts_.set(session_id, success.account)};
    return result;
}


void SessionRuntime::send_chat(const std::uint64_t session_id, const std::string_view body)
{
    const domain::SendChatMessageInput input{
        *impl_->accounts_.get(session_id),
        domain::ParticipantId{session_id},
        domain::ChatId::global(),
        body,
        domain::Timestamp::now(),
    };
    (void)impl_->send_chat_message_.execute(input);
}


HistoryResult SessionRuntime::fetch_history(const std::uint64_t session_id, const std::uint32_t limit)
{
    const domain::FetchChatHistoryInput input{*impl_->accounts_.get(session_id), domain::ChatId::global(), limit};
    const auto outcome = impl_->fetch_chat_history_.execute(input);
    if (std::holds_alternative<domain::DomainError>(outcome))
        return {};

    HistoryResult result;
    result.status = HistoryResult::Status::Ok;
    for (const auto& item : std::get<domain::FetchChatHistoryResult>(outcome).items) {
        result.items.push_back(HistoryItemDto{item.message.id, item.is_mine, item.message.author_name,
                                              item.message.body});
    }
    return result;
}


} // namespace will
