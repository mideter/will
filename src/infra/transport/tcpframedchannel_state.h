#pragma once

#include "tcpframedchannel.h"
#include "tcpframereader.h"
#include "tcpframewriter.h"

#include <cstddef>
#include <memory>
#include <string_view>
#include <vector>


namespace will {


class TcpFramedChannel::State : public std::enable_shared_from_this<State> {
public:
    State(TcpStreamSocket& stream, Strand& strand);

    Strand& strand() noexcept { return strand_; }

    void start_on_strand(FrameHandler on_frame, ClosedHandler on_closed);
    void stop_on_strand();
    void enqueue_wire_frame_on_strand(std::vector<char> wire_bytes);

private:
    void request_close();
    void handle_framing_error(std::string_view message);
    void handle_read_error(std::string_view context, const asio::error_code& ec);
    void handle_write_queue_full();
    void handle_write_error(std::string_view context, const asio::error_code& ec);

    TcpStreamSocket& stream_;
    Strand& strand_;

    std::shared_ptr<TcpFrameReader> reader_;
    std::shared_ptr<TcpFrameWriter> writer_;

    FrameHandler on_frame_;
    ClosedHandler on_closed_;

    bool closed_ = false;
    bool started_ = false;
    bool stopped_ = false;
};


} // namespace will
