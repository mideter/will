#pragma once

#include "session.h"
#include "session_id.h"

#include "identity/soul.h"

#include "infra/transport/messenger.grpc.pb.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


namespace will {


/**
 * Thread-safe registry of active gRPC sessions.
 * Owns session lifecycle, transport fan-out, and authenticated soul bindings.
 * Enforces one active session per soul.
 */
class SessionRegistry {
public:
	SessionRegistry() = default;

	SessionRegistry(const SessionRegistry&) = delete;
	SessionRegistry& operator=(const SessionRegistry&) = delete;

	std::shared_ptr<Session> register_session(grpc::ServerContext* context, Session::Stream* stream);

	void unregister_session(SessionId session_id);

	void close_session(SessionId session_id);

	void enqueue_event(SessionId session_id, const v1::ServerEvent& event);

	void broadcast_except(SessionId except_session_id, const v1::ServerEvent& event);

	void broadcast_except_soul(domain::id::Soul except_soul_id, const v1::ServerEvent& event);

	std::optional<SessionId> session_id_for_soul(domain::id::Soul soul_id) const;

	std::string_view peer_address(SessionId session_id) const;

	bool is_authenticated(SessionId session_id) const;

	std::optional<domain::id::Soul> soul_id(SessionId session_id) const;

	/** Binds soul to session. Returns the displaced session id, if any. */
	std::optional<SessionId> bind_soul(SessionId session_id, domain::id::Soul soul_id);

	void close_all_sessions();

	std::size_t count() const noexcept;

	bool at_capacity(std::size_t max_connections) const noexcept;

	/** Registers a session without transport. For unit tests only. */
	std::shared_ptr<Session> register_test_session();

private:
	static std::string peer_from_context(grpc::ServerContext* context);

	void clear_soul_binding(SessionId session_id);

	mutable std::mutex mutex_;
	std::unordered_map<std::uint64_t, std::shared_ptr<Session>> sessions_;
	std::unordered_map<std::uint64_t, SessionId> session_by_soul_;
	std::atomic<std::uint64_t> next_id_{1};
};


} // namespace will
