#pragma once

#include "tcpframedchannel.h"
#include "tcpstreamsocket.h"

#include <asio.hpp>

#include <cstdint>
#include <future>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "clientconfig.h"


namespace will {


// TCP: TcpFrame; payload is typed wire message (see wiremessage.h).
class WillClient {
public:
    WillClient();
    explicit WillClient(ClientConfig config);

    /** Sync TCP connect; starts I/O thread and {@link TcpFramedChannel}. */
    void connect();

    /** OTP phone auth + {@link BindTokenMessage} on the current TCP session (requires prior {@link #connect}). */
    void authenticate_phone(std::string_view phone, std::string_view otp_code = {});

    /** Called on the channel strand; post to another executor if needed. */
    void set_inbound_handler(std::function<void(std::vector<char>)> handler);
    void set_closed_handler(std::function<void()> handler);

    /** Sends {@link UserChatMessage} with UTF-8 body (requires prior {@link #authenticate_phone}). */
    void send(std::string_view utf8_chat_body) const;

    /** Sends {@link HistoryRequestMessage} with the given limit; returns false when limit is 0. */
    bool requestHistory(std::uint32_t limit) const;

    void shutdown() const;

    const ClientConfig& config() const noexcept;

private:
    std::vector<char> wait_for_auth_response();
    void dispatch_inbound(std::vector<char> payload);
    void dispatch_closed();

    mutable asio::io_context ioc_;
    mutable TcpStreamSocket socket_;
    TcpFramedChannel::Strand strand_{asio::make_strand(ioc_)};
    std::shared_ptr<TcpFramedChannel> channel_;
    mutable std::jthread io_thread_;

    ClientConfig config_;

    mutable std::mutex handler_mutex_;
    std::function<void(std::vector<char>)> inbound_handler_;
    std::function<void()> closed_handler_;

    std::shared_ptr<std::promise<std::vector<char>>> pending_auth_;
    mutable bool authenticated_ = false;
    mutable bool shutdown_done_ = false;
};


} // namespace will
