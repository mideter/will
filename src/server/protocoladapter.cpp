module;

#include "proto/messenger.pb.h"

#include <iostream>
#include <memory>
#include <string>
#include <string_view>

module will.server.protocoladapter;

import will.server.protocol_logic;


namespace will {


struct ProtocolAdapter::Impl {
    Impl(void* messages, void* users, void* registry, void* account_store, EnqueueEventFn enqueue,
         CloseSessionFn close, PeerAddressFn peer, BroadcastChatFn broadcast)
        : registry_(registry)
        , enqueue_(enqueue)
        , close_(close)
        , peer_(peer)
        , runtime_(messages, users, account_store, registry, reinterpret_cast<BroadcastChat>(broadcast))
    {}

    void on_client_event(std::uint64_t session_id, const v1::ClientEvent& event);
    void on_bound_event(std::uint64_t session_id, const v1::ClientEvent& event);
    void on_unbound_event(std::uint64_t session_id, const v1::ClientEvent& event);
    void handle_bind_token(std::uint64_t session_id, const v1::BindToken& token);
    void handle_user_chat(std::uint64_t session_id, const v1::ChatMessage& chat);
    void handle_history_request(std::uint64_t session_id, const v1::HistoryRequest& request);
    void send_auth_required(std::uint64_t session_id);
    void send_event(std::uint64_t session_id, const v1::ServerEvent& event);
    void close_with_protocol_error(std::uint64_t session_id, std::string_view message);
    void close_session(std::uint64_t session_id);

    void* registry_;
    EnqueueEventFn enqueue_;
    CloseSessionFn close_;
    PeerAddressFn peer_;
    SessionRuntime runtime_;
};


ProtocolAdapter::ProtocolAdapter(void* messages, void* users, void* registry, void* account_store,
                                 EnqueueEventFn enqueue, CloseSessionFn close, PeerAddressFn peer,
                                 BroadcastChatFn broadcast)
    : impl_(std::make_unique<Impl>(messages, users, registry, account_store, enqueue, close, peer, broadcast))
{}


ProtocolAdapter::~ProtocolAdapter() = default;


void ProtocolAdapter::on_client_event(const std::uint64_t session_id, const v1::ClientEvent& event)
{
    impl_->on_client_event(session_id, event);
}


void ProtocolAdapter::Impl::on_client_event(const std::uint64_t session_id, const v1::ClientEvent& event)
{
    if (runtime_.has_account(session_id)) {
        on_bound_event(session_id, event);
        return;
    }
    on_unbound_event(session_id, event);
}


void ProtocolAdapter::Impl::on_bound_event(const std::uint64_t session_id, const v1::ClientEvent& event)
{
    switch (event.event_case()) {
    case v1::ClientEvent::kChat:
        handle_user_chat(session_id, event.chat());
        return;
    case v1::ClientEvent::kHistoryRequest:
        handle_history_request(session_id, event.history_request());
        return;
    case v1::ClientEvent::kBindToken:
        handle_bind_token(session_id, event.bind_token());
        return;
    case v1::ClientEvent::EVENT_NOT_SET:
        break;
    }

    close_with_protocol_error(session_id, "Protocol error: unhandled client message type");
}


void ProtocolAdapter::Impl::on_unbound_event(const std::uint64_t session_id, const v1::ClientEvent& event)
{
    switch (event.event_case()) {
    case v1::ClientEvent::kBindToken:
        handle_bind_token(session_id, event.bind_token());
        return;
    case v1::ClientEvent::kChat:
    case v1::ClientEvent::kHistoryRequest:
        send_auth_required(session_id);
        return;
    case v1::ClientEvent::EVENT_NOT_SET:
        break;
    }

    close_with_protocol_error(session_id, "Protocol error: unhandled client message type");
}


void ProtocolAdapter::Impl::send_event(const std::uint64_t session_id, const v1::ServerEvent& event)
{
    enqueue_(registry_, session_id, event);
}


void ProtocolAdapter::Impl::close_with_protocol_error(const std::uint64_t session_id,
                                                      const std::string_view message)
{
    if (const std::string_view peer_address = peer_(registry_, session_id); !peer_address.empty())
        std::cerr << "Session " << peer_address << ": " << message << '\n';
    else
        std::cerr << "Session " << session_id << ": " << message << '\n';

    close_session(session_id);
}


void ProtocolAdapter::Impl::close_session(const std::uint64_t session_id)
{
    close_(registry_, session_id);
}


void ProtocolAdapter::Impl::handle_bind_token(const std::uint64_t session_id, const v1::BindToken& token)
{
    const BindTokenResult result = runtime_.bind_token(session_id, token.token());
    if (result.status != BindTokenResult::Status::Ok) {
        send_auth_required(session_id);
        return;
    }

    if (result.displaced_session)
        close_session(*result.displaced_session);

    v1::ServerEvent event;
    event.mutable_auth_ok();
    send_event(session_id, event);
}


void ProtocolAdapter::Impl::send_auth_required(const std::uint64_t session_id)
{
    v1::ServerEvent event;
    event.mutable_auth_required();
    send_event(session_id, event);
}


void ProtocolAdapter::Impl::handle_user_chat(const std::uint64_t session_id, const v1::ChatMessage& chat)
{
    runtime_.send_chat(session_id, chat.body());

    v1::ServerEvent event;
    event.mutable_receipt_ack();
    send_event(session_id, event);
}


void ProtocolAdapter::Impl::handle_history_request(const std::uint64_t session_id,
                                                   const v1::HistoryRequest& request)
{
    const HistoryResult history = runtime_.fetch_history(session_id, request.limit());
    if (history.status != HistoryResult::Status::Ok) {
        close_with_protocol_error(session_id, "Protocol error: invalid HistoryRequest");
        return;
    }

    for (const HistoryItemDto& item : history.items) {
        v1::ServerEvent event;
        auto* history_item = event.mutable_history_item();
        history_item->set_message_id(item.message_id);
        history_item->set_is_mine(item.is_mine);
        history_item->set_name(item.name);
        history_item->set_body(item.body);
        send_event(session_id, event);
    }

    v1::ServerEvent end_event;
    end_event.mutable_history_end();
    send_event(session_id, end_event);
}


} // namespace will
