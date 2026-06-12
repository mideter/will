#pragma once

#include "tcpframereader.h"

#include <asio.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <vector>


namespace will {


/** Async length-prefixed TCP channel: wires {@link TcpFrameReader} and {@link TcpFrameWriter}. */
class TcpFramedChannel {
public:
    using TcpSocket = asio::ip::tcp::socket;
    using Strand = asio::strand<asio::io_context::executor_type>;
    using FrameHandler = TcpFrameReader::FrameHandler;
    using ClosedHandler = std::function<void()>;

    TcpFramedChannel(TcpSocket& socket, Strand& strand, std::size_t max_outbound_queue_bytes);

    void start(FrameHandler on_frame, ClosedHandler on_closed);
    void stop();

    /** Encodes {@code app_payload} and enqueues the wire frame on {@code strand_}. */
    void send_payload(const std::vector<char>& app_payload);
    void send_payload(std::vector<char>&& app_payload);

private:
    class State;
    std::shared_ptr<State> state_;
};


} // namespace will
