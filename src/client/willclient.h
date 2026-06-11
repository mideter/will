#pragma once

#include <asio.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "clientconfig.h"
#include "wiremessage.h"


namespace will {


// TCP: TcpFrame; payload is typed wire message (see wiremessage.h).
class WillClient {
public:
    WillClient();
    explicit WillClient(ClientConfig config);

    void connect();

    /** Login + {@link BindToken} on the current TCP session. */
    void authenticate(std::string_view login, std::string_view password) const;

    /** Sends {@link UserChat} with UTF-8 body (requires prior {@link #authenticate}). */
    void send(std::string_view utf8_chat_body) const;

    /** Sends {@link HistoryRequest} with the given limit; returns false when limit is 0. */
    bool requestHistory(std::uint32_t limit) const;

    /** std::nullopt = peer closed before next frame header; otherwise decoded server message. */
    std::optional<std::unique_ptr<ServerMessage>> receiveMessage() const;

    void shutdown() const;

    const ClientConfig& config() const noexcept;

private:
    std::vector<char> receivePayload() const;

    asio::io_context ioc_;
    mutable asio::ip::tcp::socket socket_;
    ClientConfig config_;
};


} // namespace will
