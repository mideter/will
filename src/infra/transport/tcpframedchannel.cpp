#include "tcpframedchannel.h"

#include "tcpframe.h"
#include "tcpframedchannel_state.h"

#include <utility>


namespace will {


TcpFramedChannel::TcpFramedChannel(TcpStreamSocket& stream, Strand& strand,
                                   const std::size_t max_outbound_queue_bytes)
    : state_(std::make_shared<State>(stream, strand, max_outbound_queue_bytes))
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
    asio::post(state_->strand(), [state = state_] { state->stop_on_strand(); });
}


void TcpFramedChannel::send_payload(const std::vector<char>& app_payload)
{
    send_payload(std::vector<char>(app_payload));
}


void TcpFramedChannel::send_payload(std::vector<char>&& app_payload)
{
    std::vector<char> frame = TcpFrame::encode(app_payload);

    asio::post(state_->strand(), [state = state_, frame = std::move(frame)]() mutable {
        state->enqueue_frame_on_strand(std::move(frame));
    });
}


} // namespace will
