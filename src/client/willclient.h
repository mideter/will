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


/// The console's one session with the server, authenticated by the device token; the answer
/// is awaited before BindToken is sent, for it may come at once, and the inbound handler runs
/// on the reader thread. It speaks in the open, or TLS trusting only the server certificate of
/// the config. Exercises and approaches are numbered from zero; an unborn body is known by its
/// mark until it is born and named. History is not asked for with a limit of zero.
class WillClient {
public:
	WillClient();
	explicit WillClient(ClientConfig config);

	void connect();

	void set_inbound_handler(std::function<void(const v1::ServerEvent&)> handler);
	void set_closed_handler(std::function<void()> handler);

	void send(std::string_view utf8_chat_body) const;

	void ask(std::string_view addressee_name) const;
	void accept(std::string_view suppliant_name) const;

	void fulfil(std::uint64_t behest_id, std::string_view report,
				const std::vector<v1::Exercise>& performed = {}) const;

	void begin_approach(std::uint64_t behest_id, std::uint32_t exercise, std::uint32_t approach) const;
	void finish_approach(std::uint64_t behest_id, std::uint32_t weight_grams, std::uint32_t repetitions,
						 std::uint32_t strain = 0) const;
	void say_remark(std::uint64_t behest_id, std::uint32_t exercise, std::string_view body) const;
	void end_approach(std::uint64_t behest_id) const;

	void train(std::string_view title, const std::vector<v1::Exercise>& exercises) const;

	void visit(std::string_view host_name, std::string_view room = {}) const;

	void arrange(std::string_view room, bool outer) const;

	void admit(std::string_view name) const;
	void regard(std::string_view name, v1::DwellerKind kind) const;
	void list_dwellers() const;
	void list_dwellings() const;
	void list_rooms(std::string_view host_name) const;
	void list_supplications() const;
	void reject(std::string_view suppliant_name) const;

	void bear(std::uint64_t mark) const;
	void choose_father(std::string_view name) const;
	void list_lineage() const;

	bool requestHistory(std::uint32_t limit) const;

	void shutdown() const;

	const ClientConfig& config() const noexcept;
	const std::string& own_name() const noexcept { return own_name_; }

	const std::optional<std::uint64_t>& mark() const noexcept { return mark_; }

	void born(std::string name);

private:
	std::shared_ptr<grpc::ChannelCredentials> credentials() const;

	static domain::DeviceToken load_or_create_device_token(const std::string& path);
	void authenticate_device(std::string_view device_token);
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
	std::optional<std::uint64_t> mark_;
};


} // namespace will
