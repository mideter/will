#pragma once

#include "session_id.h"

#include "identity/soul.h"

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
	bool is_authenticated() const noexcept { return soul_id_.has_value(); }
	std::optional<domain::id::Soul> soul_id() const noexcept { return soul_id_; }

	bool write(const v1::ServerEvent& event);
	void request_close();

private:
	friend class SessionRegistry;

	void set_soul_id(domain::id::Soul soul_id) noexcept { soul_id_ = soul_id; }
	void clear_soul_id() noexcept { soul_id_.reset(); }

	const SessionId id_;
	grpc::ServerContext* context_;
	Stream* stream_;
	std::string peer_address_;
	std::optional<domain::id::Soul> soul_id_;
	std::mutex write_mutex_;
	std::atomic<bool> closed_{false};
};


} // namespace will
