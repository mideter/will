#pragma once

#include "clientconfig.h"
#include "values/device_token.h"

#include "infra/transport/messenger.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string_view>
#include <thread>


namespace will {


class WillClient {
public:
	WillClient();
	explicit WillClient(ClientConfig config);

	/** Opens a gRPC channel, starts Messenger.Session, and authenticates with the device token. */
	void connect();

	/** Called from the inbound reader thread. */
	void set_inbound_handler(std::function<void(const v1::ServerEvent&)> handler);
	void set_closed_handler(std::function<void()> handler);

	void send(std::string_view utf8_chat_body) const;

	void ask(std::string_view addressee_name) const;
	void accept(std::string_view suppliant_name) const;
	/** Turn the gaze to the tie with this soul, or home when the name is empty. */

	/** Fulfil a behest of the contemplated tie, with an optional report. */
	void fulfil(std::uint64_t behest_id, std::string_view report) const;

	/// Turn the gaze to the abode of this host one dwells in, or to one's own
	/// when empty; into this room of it, when given.
	void visit(std::string_view host_name, std::string_view room = {}) const;

	/// Set a room of one's abode in the outer part, or the inner one.
	void arrange(std::string_view room, bool outer) const;

	void admit(std::string_view name) const;
	void regard(std::string_view name, v1::DwellerKind kind) const;
	void list_dwellers() const;
	void list_dwellings() const;
	void list_rooms(std::string_view host_name) const;
	void list_supplications() const;
	void reject(std::string_view suppliant_name) const;

	/** Sends HistoryRequest; returns false when limit is 0. */
	bool requestHistory(std::uint32_t limit) const;

	void shutdown() const;

	const ClientConfig& config() const noexcept;
	const std::string& own_name() const noexcept { return own_name_; }

private:
	static domain::DeviceToken load_or_create_device_token(const std::string& path);
	void authenticate_device(std::string_view device_token);
	/** Must be called before BindToken is sent: the reply may arrive at once. */
	std::future<v1::ServerEvent> expect_auth_response();
	std::shared_ptr<std::promise<v1::ServerEvent>> take_pending_auth();
	void dispatch_inbound(const v1::ServerEvent& event);
	void dispatch_closed();
	void reader_loop();
	bool write_event(const v1::ClientEvent& event) const;

	ClientConfig config_;

	mutable std::shared_ptr<grpc::Channel> channel_;
	mutable std::unique_ptr<v1::Messenger::Stub> stub_;
	mutable std::unique_ptr<grpc::ClientContext> context_;
	using Stream = grpc::ClientReaderWriter<v1::ClientEvent, v1::ServerEvent>;
	mutable std::shared_ptr<Stream> stream_;

	mutable std::mutex write_mutex_;
	mutable std::jthread reader_thread_;

	mutable std::mutex handler_mutex_;
	std::function<void(const v1::ServerEvent&)> inbound_handler_;
	std::function<void()> closed_handler_;

	std::mutex auth_mutex_;
	std::shared_ptr<std::promise<v1::ServerEvent>> pending_auth_;
	mutable bool authenticated_ = false;
	mutable bool shutdown_done_ = false;
	mutable std::atomic<bool> closed_{false};
	std::string own_name_;
};


} // namespace will
