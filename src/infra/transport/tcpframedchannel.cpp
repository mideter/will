#include "tcpframedchannel.h"

#include "tcpframe.h"
#include "tcpframedchannel_state.h"

#include <utility>


namespace will {


TcpFramedChannel::TcpFramedChannel(TcpStreamSocket& stream, Strand& strand)
    : state_(std::make_shared<State>(stream, strand))
{}


void TcpFramedChannel::start(FrameHandler on_frame, ClosedHandler on_closed)
{
    asio::dispatch(state_->strand(), [state = state_, on_frame = std::move(on_frame),
                                      on_closed = std::move(on_closed)]() mutable {
        state->start_on_strand(std::move(on_frame), std::move(on_closed));
    });
}


void TcpFramedChannel::stop()
{
    asio::dispatch(state_->strand(), [state = state_] { state->stop_on_strand(); });
}


void TcpFramedChannel::enqueue_wire_frame(std::vector<char>&& wire_bytes)
{
    asio::post(state_->strand(), [state = state_, wire_bytes = std::move(wire_bytes)]() mutable {
        state->enqueue_wire_frame_on_strand(std::move(wire_bytes));
    });
}


void TcpFramedChannel::send_payload(const std::vector<char>& payload)
{
    send_payload(std::vector<char>(payload));
}


void TcpFramedChannel::send_payload(std::vector<char>&& payload)
{
    std::vector<char> wire_bytes = TcpFrame::encode(payload);

    asio::post(state_->strand(), [state = state_, wire_bytes = std::move(wire_bytes)]() mutable {
        state->enqueue_wire_frame_on_strand(std::move(wire_bytes));
    });
}


} // namespace will
