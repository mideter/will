#include "willclient.h"

#include "clientconfigvalidator.h"

#include <fstream>
#include <future>
#include <stdexcept>
#include <utility>


namespace will {


WillClient::WillClient() = default;


WillClient::WillClient(ClientConfig config)
	: config_(ClientConfigValidator::accept(std::move(config)))
{}


const ClientConfig& WillClient::config() const noexcept
{
	return config_;
}


void WillClient::set_inbound_handler(std::function<void(const v1::ServerEvent&)> handler)
{
	std::lock_guard lock(handler_mutex_);
	inbound_handler_ = std::move(handler);
}


void WillClient::set_closed_handler(std::function<void()> handler)
{
	std::lock_guard lock(handler_mutex_);
	closed_handler_ = std::move(handler);
}


void WillClient::dispatch_inbound(const v1::ServerEvent& event)
{
	std::function<void(const v1::ServerEvent&)> handler;
	{
		std::lock_guard lock(handler_mutex_);
		handler = inbound_handler_;
	}

	if (handler)
		handler(event);
}


void WillClient::dispatch_closed()
{
	if (const auto pending = take_pending_auth()) {
		pending->set_exception(
			std::make_exception_ptr(std::runtime_error("Will protocol: unexpected end of stream")));
	}

	std::function<void()> handler;
	{
		std::lock_guard lock(handler_mutex_);
		handler = closed_handler_;
	}

	if (handler)
		handler();
}


std::future<v1::ServerEvent> WillClient::expect_auth_response()
{
	auto pending = std::make_shared<std::promise<v1::ServerEvent>>();
	std::future<v1::ServerEvent> response = pending->get_future();

	std::lock_guard lock(auth_mutex_);
	pending_auth_ = std::move(pending);

	return response;
}


std::shared_ptr<std::promise<v1::ServerEvent>> WillClient::take_pending_auth()
{
	std::lock_guard lock(auth_mutex_);
	return std::exchange(pending_auth_, nullptr);
}


bool WillClient::write_event(const v1::ClientEvent& event) const
{
	std::lock_guard lock(write_mutex_);

	if (!stream_ || closed_.load())
		return false;

	return stream_->Write(event);
}


void WillClient::reader_loop()
{
	v1::ServerEvent event;
	while (stream_ && stream_->Read(&event)) {
		if (const auto pending = take_pending_auth()) {
			pending->set_value(event);
			continue;
		}

		dispatch_inbound(event);
	}

	closed_.store(true);
	dispatch_closed();
}


domain::DeviceToken WillClient::load_or_create_device_token(const std::string& path)
{
	{
		std::ifstream in(path);
		if (in) {
			std::string token;
			in >> token;
			if (const auto parsed = domain::DeviceToken::parse(token))
				return *parsed;
		}
	}

	const domain::DeviceToken token = domain::DeviceToken::generate();

	std::ofstream out(path, std::ios::trunc);
	if (!out)
		throw std::runtime_error("WillClient: failed to write " + path);

	out << token.text();
	if (!out)
		throw std::runtime_error("WillClient: failed to persist device token");

	return token;
}


void WillClient::connect()
{
	if (stream_)
		throw std::logic_error("WillClient: already connected");

	const domain::DeviceToken device_token = load_or_create_device_token(config_.device_token_path);

	const std::string target = config_.host + ":" + std::to_string(config_.port);
	channel_ = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
	stub_ = v1::Messenger::NewStub(channel_);
	context_ = std::make_unique<grpc::ClientContext>();
	stream_ = stub_->Session(context_.get());
	if (!stream_)
		throw std::runtime_error("WillClient: failed to open Session stream");

	reader_thread_ = std::jthread([this] { reader_loop(); });
	authenticate_device(device_token.text());
}


void WillClient::authenticate_device(const std::string_view device_token)
{
	if (!stream_)
		throw std::logic_error("WillClient: not connected");

	if (authenticated_)
		throw std::logic_error("WillClient: already authenticated");

	v1::ClientEvent event;
	event.mutable_bind_token()->set_token(std::string(device_token));
	std::future<v1::ServerEvent> pending_response = expect_auth_response();
	if (!write_event(event)) {
		take_pending_auth();
		throw std::runtime_error("Will protocol: failed to send BindToken");
	}

	const v1::ServerEvent response = pending_response.get();
	if (response.has_auth_required())
		throw std::runtime_error("Will protocol: device authentication failed");

	if (!response.has_auth_ok())
		throw std::runtime_error("Will protocol: expected AuthOk");

	own_name_ = response.auth_ok().name();
	authenticated_ = true;
}


void WillClient::send(const std::string_view utf8_chat_body) const
{
	if (!authenticated_)
		throw std::logic_error("WillClient: not authenticated");

	v1::ClientEvent event;
	event.mutable_chat()->set_body(std::string(utf8_chat_body));

	if (!write_event(event))
		throw std::runtime_error("Will protocol: failed to send chat message");
}


void WillClient::ask(const std::string_view addressee_name) const
{
	if (!authenticated_)
		throw std::logic_error("WillClient: not authenticated");

	v1::ClientEvent event;
	event.mutable_supplicate()->set_addressee_name(std::string{addressee_name});
	if (!write_event(event))
		throw std::runtime_error("Will protocol: failed to send Supplicate");
}


void WillClient::accept(const std::string_view suppliant_name) const
{
	if (!authenticated_)
		throw std::logic_error("WillClient: not authenticated");

	v1::ClientEvent event;
	event.mutable_accept_supplication()->set_suppliant_name(std::string{suppliant_name});
	if (!write_event(event))
		throw std::runtime_error("Will protocol: failed to send AcceptSupplication");
}


void WillClient::fulfil(const std::uint64_t behest_id, const std::string_view report) const
{
	if (!authenticated_)
		throw std::logic_error("WillClient: not authenticated");

	v1::ClientEvent event;
	auto* msg = event.mutable_fulfil();
	msg->set_behest_id(behest_id);
	msg->set_report(std::string{report});
	if (!write_event(event))
		throw std::runtime_error("Will protocol: failed to send Fulfil");
}


void WillClient::visit(const std::string_view host_name, const std::string_view room) const
{
	if (!authenticated_)
		throw std::logic_error("WillClient: not authenticated");

	v1::ClientEvent event;
	event.mutable_turn()->set_abode_of(std::string{host_name});
	event.mutable_turn()->set_room(std::string{room});
	if (!write_event(event))
		throw std::runtime_error("Will protocol: failed to send Turn");
}


void WillClient::arrange(const std::string_view room, const bool outer) const
{
	if (!authenticated_)
		throw std::logic_error("WillClient: not authenticated");

	v1::ClientEvent event;
	event.mutable_arrange()->set_room(std::string{room});
	event.mutable_arrange()->set_part(outer ? v1::OUTER : v1::INNER);
	if (!write_event(event))
		throw std::runtime_error("Will protocol: failed to send Arrange");
}


void WillClient::admit(const std::string_view name) const
{
	if (!authenticated_)
		throw std::logic_error("WillClient: not authenticated");

	v1::ClientEvent event;
	event.mutable_admit()->set_name(std::string{name});
	if (!write_event(event))
		throw std::runtime_error("Will protocol: failed to send Admit");
}


void WillClient::regard(const std::string_view name, const v1::DwellerKind kind) const
{
	if (!authenticated_)
		throw std::logic_error("WillClient: not authenticated");

	v1::ClientEvent event;
	auto* msg = event.mutable_regard();
	msg->set_name(std::string{name});
	msg->set_kind(kind);
	if (!write_event(event))
		throw std::runtime_error("Will protocol: failed to send Regard");
}


void WillClient::list_dwellers() const
{
	if (!authenticated_)
		throw std::logic_error("WillClient: not authenticated");

	v1::ClientEvent event;
	event.mutable_list_dwellers();
	if (!write_event(event))
		throw std::runtime_error("Will protocol: failed to send ListDwellers");
}


void WillClient::list_dwellings() const
{
	if (!authenticated_)
		throw std::logic_error("WillClient: not authenticated");

	v1::ClientEvent event;
	event.mutable_list_dwellings();
	if (!write_event(event))
		throw std::runtime_error("Will protocol: failed to send ListDwellings");
}


void WillClient::list_rooms(const std::string_view host_name) const
{
	if (!authenticated_)
		throw std::logic_error("WillClient: not authenticated");

	v1::ClientEvent event;
	event.mutable_list_rooms()->set_abode_of(std::string{host_name});
	if (!write_event(event))
		throw std::runtime_error("Will protocol: failed to send ListRooms");
}


void WillClient::list_supplications() const
{
	if (!authenticated_)
		throw std::logic_error("WillClient: not authenticated");

	v1::ClientEvent event;
	event.mutable_list_supplications();
	if (!write_event(event))
		throw std::runtime_error("Will protocol: failed to send ListSupplications");
}


void WillClient::reject(const std::string_view suppliant_name) const
{
	if (!authenticated_)
		throw std::logic_error("WillClient: not authenticated");

	v1::ClientEvent event;
	event.mutable_reject_supplication()->set_suppliant_name(std::string{suppliant_name});
	if (!write_event(event))
		throw std::runtime_error("Will protocol: failed to send RejectSupplication");
}


bool WillClient::requestHistory(const std::uint32_t limit) const
{
	if (limit == 0)
		return false;

	if (!authenticated_)
		throw std::logic_error("WillClient: not authenticated");

	v1::ClientEvent event;
	event.mutable_history_request()->set_limit(limit);
	if (!write_event(event))
		throw std::runtime_error("Will protocol: failed to send HistoryRequest");

	return true;
}


void WillClient::shutdown() const
{
	if (shutdown_done_)
		return;

	shutdown_done_ = true;
	closed_.store(true);

	{
		std::lock_guard lock(write_mutex_);
		if (stream_)
			stream_->WritesDone();
	}

	if (context_)
		context_->TryCancel();

	if (reader_thread_.joinable())
		reader_thread_.join();

	stream_.reset();
	stub_.reset();
	context_.reset();
	channel_.reset();
}


} // namespace will
