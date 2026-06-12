#include "willclient.h"

#include "clientconfigvalidator.h"

#include <future>
#include <stdexcept>
#include <utility>

#include "wiremessage_client.h"
#include "wiremessage_codec.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"


namespace will {


WillClient::WillClient()
    : socket_(ioc_)
{}


WillClient::WillClient(ClientConfig config)
    : socket_(ioc_)
    , config_(ClientConfigValidator::accept(std::move(config)))
{}


const ClientConfig& WillClient::config() const noexcept
{
    return config_;
}


void WillClient::set_inbound_handler(std::function<void(std::vector<char>)> handler)
{
    std::lock_guard lock(handler_mutex_);
    inbound_handler_ = std::move(handler);
}


void WillClient::set_closed_handler(std::function<void()> handler)
{
    std::lock_guard lock(handler_mutex_);
    closed_handler_ = std::move(handler);
}


void WillClient::dispatch_inbound(std::vector<char> payload)
{
    std::function<void(std::vector<char>)> handler;
    {
        std::lock_guard lock(handler_mutex_);
        handler = inbound_handler_;
    }

    if (handler)
        handler(std::move(payload));
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


void WillClient::connect()
{
    if (channel_)
        throw std::logic_error("WillClient: already connected");

    socket_.connect(config_.host, config_.port);

    channel_ = std::make_shared<TcpFramedChannel>(socket_, strand_);
    channel_->start(
        [this](std::vector<char> payload) {
            if (pending_auth_) {
                try {
                    pending_auth_->set_value(std::move(payload));
                } catch (const std::future_error&) {
                }
                pending_auth_.reset();
                return;
            }

            dispatch_inbound(std::move(payload));
        },
        [this] { dispatch_closed(); });

    io_thread_ = std::jthread([this] { ioc_.run(); });
}


void WillClient::authenticate(const std::string_view login, const std::string_view password)
{
    if (!channel_)
        throw std::logic_error("WillClient: not connected");

    if (authenticated_)
        throw std::logic_error("WillClient: already authenticated");

    pending_auth_ = std::make_shared<std::promise<std::vector<char>>>();
    std::future<std::vector<char>> login_future = pending_auth_->get_future();

    channel_->send_payload(
        WireMessageCodec::encode(LoginRequestMessage{std::string(login), std::string(password)}));

    const std::vector<char> response = login_future.get();

    const auto message = WireMessageCodec::decode_server(response);
    const auto* parsed = dynamic_cast<const LoginResponseMessage*>(message.get());
    if (!parsed || !parsed->success())
        throw std::runtime_error("Will protocol: login failed");

    channel_->send_payload(WireMessageCodec::encode(BindTokenMessage{parsed->token()}));
    authenticated_ = true;
}


void WillClient::send(const std::string_view utf8_chat_body) const
{
    if (!authenticated_)
        throw std::logic_error("WillClient: not authenticated");

    channel_->send_payload(WireMessageCodec::encode(UserChatMessage{std::string(utf8_chat_body)}));
}


bool WillClient::requestHistory(const std::uint32_t limit) const
{
    if (limit == 0)
        return false;

    if (!authenticated_)
        throw std::logic_error("WillClient: not authenticated");

    channel_->send_payload(WireMessageCodec::encode(HistoryRequestMessage{limit}));
    return true;
}


void WillClient::shutdown() const
{
    if (shutdown_done_)
        return;

    shutdown_done_ = true;

    if (channel_)
        channel_->stop();

    socket_.shutdown_and_close();
    ioc_.stop();

    if (io_thread_.joinable())
        io_thread_.join();
}


} // namespace will
