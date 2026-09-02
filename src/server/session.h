#pragma once

#include "session_id.h"

#include "ids/user_id.h"

#include "infra/transport/messenger.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>


namespace will {


class SessionRegistry;


class Session {
public:
    using Stream = grpc::ServerReaderWriter<v1::ServerEvent, v1::ClientEvent>;

    Session(SessionId id, grpc::ServerContext* context, Stream* stream, std::string peer_address);

    SessionId id() const noexcept { return id_; }
    std::string_view peer_address() const noexcept { return peer_address_; }
    bool closed() const noexcept { return closed_.load(); }
    bool is_authenticated() const noexcept { return user_id_.has_value(); }
    std::optional<domain::UserId> user_id() const noexcept { return user_id_; }

    bool write(const v1::ServerEvent& event);
    void request_close();

private:
    friend class SessionRegistry;

    void set_user_id(domain::UserId user_id) noexcept { user_id_ = user_id; }
    void clear_user_id() noexcept { user_id_.reset(); }

    const SessionId id_;
    grpc::ServerContext* context_;
    Stream* stream_;
    std::string peer_address_;
    std::optional<domain::UserId> user_id_;
    std::mutex write_mutex_;
    std::atomic<bool> closed_{false};
};


} // namespace will
