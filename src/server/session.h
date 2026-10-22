#pragma once

#include "session_id.h"

#include "identity/vessel.h"

#include "infra/transport/messenger.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <thread>
#include <optional>
#include <string>
#include <string_view>


namespace will {


class SessionRegistry;


/// Session — one client's stream. What the server tells the client goes into the
/// session's own queue and is written out by its own writer, so that whoever acts
/// never waits for another's network: a client that reads slowly or not at all holds
/// up only itself, and is closed when it falls MaxPending events behind. The writer is
/// stopped and waited for before the stream goes away.
class Session {
public:
	using Stream = grpc::ServerReaderWriter<v1::ServerEvent, v1::ClientEvent>;

	static constexpr std::size_t MaxPending = 1024;

	Session(SessionId id, grpc::ServerContext* context, Stream* stream, std::string peer_address);
	~Session();

	Session(const Session&) = delete;
	Session& operator=(const Session&) = delete;

	SessionId id() const noexcept { return id_; }
	std::string_view peer_address() const noexcept { return peer_address_; }
	bool closed() const noexcept { return closed_.load(); }
	bool is_authenticated() const noexcept { return vessel_id_.has_value(); }
	std::optional<domain::id::Vessel> vessel_id() const noexcept { return vessel_id_; }

	bool write(const v1::ServerEvent& event);
	void request_close();
	void finish();

private:
	friend class SessionRegistry;

	void write_out();

	void set_vessel_id(domain::id::Vessel vessel_id) noexcept { vessel_id_ = vessel_id; }
	void clear_vessel_id() noexcept { vessel_id_.reset(); }

	const SessionId id_;
	grpc::ServerContext* context_;
	Stream* stream_;
	std::string peer_address_;
	std::optional<domain::id::Vessel> vessel_id_;
	std::mutex context_mutex_;
	std::atomic<bool> closed_{false};

	std::mutex pending_mutex_;
	std::condition_variable pending_changed_;
	std::deque<v1::ServerEvent> pending_;
	bool finishing_ = false;
	std::thread writer_;  // last: started once the rest is ready
};


} // namespace will
