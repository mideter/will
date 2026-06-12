#include "willclient.h"

#include "clientconfigvalidator.h"

#include <future>
#include <iostream>
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


std::vector<char> WillClient::wait_for_auth_response()
{
    pending_auth_ = std::make_shared<std::promise<std::vector<char>>>();
    return pending_auth_->get_future().get();
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


void WillClient::authenticate_phone(const std::string_view phone, const std::string_view otp_code)
{
    if (!channel_)
        throw std::logic_error("WillClient: not connected");

    if (authenticated_)
        throw std::logic_error("WillClient: already authenticated");

    channel_->send_payload(
        WireMessageCodec::encode(OtpPhoneRequestMessage{std::string(phone)}));

    const std::vector<char> otp_request_response = wait_for_auth_response();
    const auto otp_request_message = WireMessageCodec::decode_server(otp_request_response);
    if (const auto* failure = dynamic_cast<const OtpVerifyResponseMessage*>(otp_request_message.get())) {
        if (!failure->success())
            throw std::runtime_error("Will protocol: OTP request failed");
    } else if (dynamic_cast<const OtpSentMessage*>(otp_request_message.get()) == nullptr) {
        throw std::runtime_error("Will protocol: expected OtpSent");
    }

    std::string code;
    if (otp_code.empty()) {
        std::cerr << "Enter OTP code: ";
        if (!std::getline(std::cin, code) || code.empty())
            throw std::runtime_error("OTP code required");
    } else {
        code.assign(otp_code);
    }

    channel_->send_payload(WireMessageCodec::encode(OtpCodeSubmitMessage{code}));

    const std::vector<char> verify_response = wait_for_auth_response();
    const auto verify_message = WireMessageCodec::decode_server(verify_response);
    const auto* parsed = dynamic_cast<const OtpVerifyResponseMessage*>(verify_message.get());
    if (!parsed || !parsed->success())
        throw std::runtime_error("Will protocol: OTP verification failed");

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
