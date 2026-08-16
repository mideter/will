#include "willclient.h"

#include "clientconfigvalidator.h"

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
    if (pending_auth_) {
        try {
            pending_auth_->set_exception(
                std::make_exception_ptr(std::runtime_error("Will protocol: unexpected end of stream")));
        } catch (const std::future_error&) {
        }
        pending_auth_.reset();
    }

    std::function<void()> handler;
    {
        std::lock_guard lock(handler_mutex_);
        handler = closed_handler_;
    }

    if (handler)
        handler();
}


v1::ServerEvent WillClient::wait_for_auth_response()
{
    pending_auth_ = std::make_shared<std::promise<v1::ServerEvent>>();
    return pending_auth_->get_future().get();
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
        if (pending_auth_) {
            try {
                pending_auth_->set_value(event);
            } catch (const std::future_error&) {
            }
            pending_auth_.reset();
            continue;
        }

        dispatch_inbound(event);
    }

    closed_.store(true);
    dispatch_closed();
}


void WillClient::connect()
{
    if (stream_)
        throw std::logic_error("WillClient: already connected");

    const std::string target = config_.host + ":" + std::to_string(config_.port);
    channel_ = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    stub_ = v1::Messenger::NewStub(channel_);
    context_ = std::make_unique<grpc::ClientContext>();
    stream_ = stub_->Session(context_.get());
    if (!stream_)
        throw std::runtime_error("WillClient: failed to open Session stream");

    reader_thread_ = std::jthread([this] { reader_loop(); });
}


void WillClient::authenticate_device(const std::string_view device_token)
{
    if (!stream_)
        throw std::logic_error("WillClient: not connected");

    if (authenticated_)
        throw std::logic_error("WillClient: already authenticated");

    v1::ClientEvent event;
    event.mutable_bind_token()->set_token(std::string(device_token));
    if (!write_event(event))
        throw std::runtime_error("Will protocol: failed to send BindToken");

    const v1::ServerEvent response = wait_for_auth_response();
    if (response.has_auth_required())
        throw std::runtime_error("Will protocol: device authentication failed");

    if (!response.has_auth_ok())
        throw std::runtime_error("Will protocol: expected AuthOk");

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
