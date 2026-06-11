#include "willclient.h"

#include "clientconfigvalidator.h"

#include <arpa/inet.h>

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include "wiremessage_client.h"
#include "wiremessage_codec.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"
#include "tcpframe.h"


namespace will {


namespace {


void write_all(asio::ip::tcp::socket& socket, const void* data, std::size_t len)
{
    asio::write(socket, asio::buffer(data, len));
}


void send_payload(asio::ip::tcp::socket& socket, const std::vector<char>& payload)
{
    const std::vector<char> frame = TcpFrame::encode(payload);
    write_all(socket, frame.data(), frame.size());
}


/** Exactly {@code len} bytes, or EOF before/on first byte ({@code true}), or protocol error after partial read. */
bool read_exact_or_eof_before_first_byte(asio::ip::tcp::socket& socket, unsigned char* data,
                                         std::size_t len)
{
    asio::error_code ec;
    std::size_t got = 0;

    while (got < len) {
        const std::size_t n = socket.read_some(asio::buffer(data + got, len - got), ec);

        if (ec == asio::error::eof) {
            if (got == 0)
                return true;

            throw std::runtime_error("Will protocol: connection closed mid-frame");
        }

        if (ec)
            throw std::system_error(ec);

        got += n;
    }

    return false;
}


void read_exact(asio::ip::tcp::socket& socket, unsigned char* data, std::size_t len)
{
    if (read_exact_or_eof_before_first_byte(socket, data, len))
        throw std::runtime_error("Will protocol: unexpected end of stream mid-frame");
}


} // namespace


WillClient::WillClient()
    : socket_(ioc_)
{}


WillClient::WillClient(ClientConfig config)
    : socket_(ioc_)
    , config_(ClientConfigValidator::accept(std::move(config)))
{
    connect();
    authenticate(config_.login, config_.password);
}


const ClientConfig& WillClient::config() const noexcept
{
    return config_;
}


void WillClient::connect()
{
    socket_.connect(
        asio::ip::tcp::endpoint(asio::ip::make_address_v4(config_.host), config_.port));
    socket_.set_option(asio::socket_base::keep_alive(true));
}


void WillClient::authenticate(const std::string_view login, const std::string_view password) const
{
    send_payload(socket_, WireMessageCodec::encode(LoginRequestMessage{std::string(login), std::string(password)}));

    const std::vector<char> response = receivePayload();
    const auto message = WireMessageCodec::decode_server(response);
    const auto* parsed = dynamic_cast<const LoginResponseMessage*>(message.get());
    if (!parsed || !parsed->success())
        throw std::runtime_error("Will protocol: login failed");

    send_payload(socket_, WireMessageCodec::encode(BindTokenMessage{parsed->token()}));
}


std::vector<char> WillClient::receivePayload() const
{
    std::array<unsigned char, 4> len_bytes{};
    if (read_exact_or_eof_before_first_byte(socket_, len_bytes.data(), len_bytes.size()))
        throw std::runtime_error("Will protocol: unexpected end of stream");

    const std::uint32_t len_u32 = TcpFrame::read_u32_be(len_bytes);
    const std::size_t plen = static_cast<std::size_t>(len_u32);

    if (plen > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("Will protocol: frame exceeds TcpFrame::MaxPayloadBytes");

    if (plen == 0)
        throw std::runtime_error("Will protocol: empty typed payload is invalid");

    std::vector<char> payload(plen);
    read_exact(socket_, reinterpret_cast<unsigned char*>(payload.data()), plen);
    return payload;
}


void WillClient::send(std::string_view utf8_chat_body) const
{
    send_payload(socket_, WireMessageCodec::encode(UserChatMessage{std::string(utf8_chat_body)}));
}


bool WillClient::requestHistory(const std::uint32_t limit) const
{
    if (limit == 0)
        return false;

    send_payload(socket_, WireMessageCodec::encode(HistoryRequestMessage{limit}));
    return true;
}


std::optional<std::vector<char>> WillClient::receiveFrame() const
{
    std::array<unsigned char, 4> len_bytes{};
    if (read_exact_or_eof_before_first_byte(socket_, len_bytes.data(), len_bytes.size()))
        return std::nullopt;

    const std::uint32_t len_u32 = TcpFrame::read_u32_be(len_bytes);
    const std::size_t plen = static_cast<std::size_t>(len_u32);

    if (plen > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("Will protocol: frame exceeds TcpFrame::MaxPayloadBytes");

    if (plen == 0)
        throw std::runtime_error("Will protocol: empty typed payload is invalid");

    std::vector<char> payload(plen);
    read_exact(socket_, reinterpret_cast<unsigned char*>(payload.data()), plen);
    return payload;
}


void WillClient::shutdown() const
{
    asio::error_code ignored;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
}


} // namespace will
