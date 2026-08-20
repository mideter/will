module;

#include "proto/messenger.pb.h"

#include <cstdint>
#include <memory>
#include <string_view>

export module will.server.protocoladapter;

export namespace will {


using EnqueueEventFn = void (*)(void* registry, std::uint64_t session_id, const v1::ServerEvent& event);
using CloseSessionFn = void (*)(void* registry, std::uint64_t session_id);
using PeerAddressFn = std::string_view (*)(void* registry, std::uint64_t session_id);
using BroadcastChatFn = void (*)(void* registry, std::uint64_t except_session, std::string_view name,
                                 std::string_view body);


/** Maps client protobuf events to domain use cases and enqueues outbound events. */
class ProtocolAdapter {
public:
    ProtocolAdapter(void* messages, void* users, void* registry, void* account_store, EnqueueEventFn enqueue,
                    CloseSessionFn close, PeerAddressFn peer, BroadcastChatFn broadcast);
    ~ProtocolAdapter();

    ProtocolAdapter(const ProtocolAdapter&) = delete;
    ProtocolAdapter& operator=(const ProtocolAdapter&) = delete;

    void on_client_event(std::uint64_t session_id, const v1::ClientEvent& event);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};


} // namespace will
