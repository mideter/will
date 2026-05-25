#include "willclient.h"

#include <arpa/inet.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include "willmessage.h"


namespace will {


namespace {


void write_all(asio::ip::tcp::socket& socket, const void* data, std::size_t len)
{
    asio::write(socket, asio::buffer(data, len));
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


WillClient::WillClient(const ClientConfig& config)
    : socket_(ioc_)
    , config_(config)
{
    connect();
}


const ClientConfig& WillClient::config() const noexcept
{
    return config_;
}


void WillClient::connect()
{
    socket_.connect(config_.server_address().endpoint());
    socket_.set_option(asio::socket_base::keep_alive(true));
}


void WillClient::send(std::string_view utf8_chat_body) const
{
    const std::vector<char> payload = WillMessage::encode_user_chat(utf8_chat_body);

    if (payload.size() > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("message exceeds TcpFrame::MaxPayloadBytes");

    unsigned char header[4];
    TcpFrame::append_u32_be(header, payload.size());
    write_all(socket_, header, sizeof(header));
    write_all(socket_, payload.data(), payload.size());
}


std::optional<InboundMessage> WillClient::receiveMessage() const
{
    unsigned char len_bytes[4];
    if (read_exact_or_eof_before_first_byte(socket_, len_bytes, sizeof(len_bytes)))
        return std::nullopt;

    const std::uint32_t len_u32 = TcpFrame::read_u32_be(len_bytes);
    const std::size_t plen = static_cast<std::size_t>(len_u32);

    if (plen > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("Will protocol: frame exceeds TcpFrame::MaxPayloadBytes");

    if (plen == 0)
        throw std::runtime_error("Will protocol: empty typed payload is invalid");

    std::vector<char> payload(plen);
    read_exact(socket_, reinterpret_cast<unsigned char*>(payload.data()), plen);

    if (WillMessage::is_server_receipt_ack(payload))
        return InboundMessage{std::in_place_type<ServerReceiptAck>};

    if (WillMessage::is_user_chat(payload))
        return InboundMessage{std::in_place_type<std::string>, std::string(payload.begin() + 1, payload.end())};

    throw std::runtime_error("Will protocol: unknown message type");
}


void WillClient::shutdown() const
{
    asio::error_code ignored;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
}


} // namespace will
