#pragma once

#include "tcpframereader.h"
#include "tcpstreamsocket.h"

#include <asio.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <vector>


namespace will {


/** Async length-prefixed TCP channel: wires {@link TcpFrameReader} and {@link TcpFrameWriter}. */
class TcpFramedChannel {
public:
    using Strand = asio::strand<asio::io_context::executor_type>;
    using FrameHandler = TcpFrameReader::FrameHandler;
    using ClosedHandler = std::function<void()>;

    TcpFramedChannel(TcpStreamSocket& stream, Strand& strand);

    void start(FrameHandler on_frame, ClosedHandler on_closed);
    void stop();

    /** Encodes {@code payload} and enqueues the wire frame on {@code strand_}. */
    void send_payload(const std::vector<char>& payload);
    void send_payload(std::vector<char>&& payload);

    /** Enqueues pre-encoded wire bytes on {@code strand_}. */
    void enqueue_wire_frame(std::vector<char>&& wire_bytes);

private:
    class State;
    std::shared_ptr<State> state_;
};


} // namespace will
