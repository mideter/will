#include "client.h"

#include <cstdint>
#include <stdexcept>

#include "clientconnection.h"
#include "listensocketstopsignals.h"
#include "willprotocol.h"


namespace will {


Client::Client(ClientConnection connection)
    : connection_(std::move(connection))
    , chat_peer_signal_slot_(ListenSocketStopSignals::register_chat_peer_fd(connection_.socket_fd()))
{
    if (chat_peer_signal_slot_ < 0)
        throw std::runtime_error{"ListenSocketStopSignals chat peer FD registry full"};
}


const ClientAddress& Client::address() const noexcept
{
    return connection_.address();
}


void Client::shutdown()
{
    connection_.shutdown();

    const int slot = chat_peer_signal_slot_;
    if (slot >= 0) {
        ListenSocketStopSignals::unregister_chat_peer_fd(slot);
        chat_peer_signal_slot_ = -1;
    }
}


bool Client::recv_exact_relaxed_eof_before_first_byte(const ClientConnection& from,
                                                       char* data,
                                                       std::size_t len)
{
    std::size_t got = 0;
    while (got < len) {
        std::size_t chunk = 0;
        if (!from.recv_some(data + got, len - got, chunk)) {
            if (got != 0)
                throw std::runtime_error{"Will frame: connection closed mid-frame"};
            return false;
        }
        got += chunk;
    }
    return true;
}


bool Client::recv_frame(std::vector<char>& payload_out) const
{
    char header_buf[4];
    if (!recv_exact_relaxed_eof_before_first_byte(connection_, header_buf, sizeof(header_buf)))
        return false;

    const unsigned char* const header_u = reinterpret_cast<const unsigned char*>(header_buf);
    const std::uint32_t len_u32 = TcpFrame::read_u32_be(header_u);
    const auto plen = static_cast<std::size_t>(len_u32);
    
    if (plen > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error{"Will frame: frame exceeds TcpFrame::MaxPayloadBytes"};

    payload_out.assign(plen, '\0');
    if (!payload_out.empty()) {
        if (!recv_exact_relaxed_eof_before_first_byte(connection_, payload_out.data(), payload_out.size()))
            throw std::runtime_error{"Will frame: connection closed mid-frame"};
    }
    return true;
}


void Client::send_frame(const char* payload, std::size_t payload_len) const
{
    unsigned char header_buf[4];
    TcpFrame::append_u32_be(header_buf, payload_len);
    connection_.send_all(reinterpret_cast<const char*>(header_buf), sizeof(header_buf));
    if (payload_len != 0)
        connection_.send_all(payload, payload_len);
}


void Client::send_frame(const std::vector<char>& payload) const
{
    send_frame(payload.data(), payload.size());
}


} // namespace will
