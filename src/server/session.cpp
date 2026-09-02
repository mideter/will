#include "session.h"

#include <utility>


namespace will {


Session::Session(const SessionId id, grpc::ServerContext* context, Stream* stream, std::string peer_address)
    : id_(id)
    , context_(context)
    , stream_(stream)
    , peer_address_(std::move(peer_address))
{}


bool Session::write(const v1::ServerEvent& event)
{
    std::lock_guard lock(write_mutex_);
    if (closed_.load() || stream_ == nullptr)
        return false;

    return stream_->Write(event);
}


void Session::request_close()
{
    grpc::ServerContext* context = nullptr;
    {
        std::lock_guard lock(write_mutex_);
        if (closed_.exchange(true))
            return;
        context = context_;
        context_ = nullptr;
    }

    if (context != nullptr)
        context->TryCancel();
}


} // namespace will
