#include "session.h"

#include <utility>


namespace will {


Session::Session(const SessionId id, grpc::ServerContext* context, Stream* stream, std::string peer_address)
	: id_(id)
	, context_(context)
	, stream_(stream)
	, peer_address_(std::move(peer_address))
{
	if (stream_ != nullptr)
		writer_ = std::thread([this] { write_out(); });
}


Session::~Session()
{
	finish();
}


bool Session::write(const v1::ServerEvent& event)
{
	if (closed_.load() || stream_ == nullptr)
		return false;

	{
		std::lock_guard lock(pending_mutex_);
		if (finishing_ || pending_.size() >= MaxPending)
			return false;
		pending_.push_back(event);
	}
	pending_changed_.notify_one();
	return true;
}


void Session::write_out()
{
	for (;;) {
		v1::ServerEvent event;
		{
			std::unique_lock lock(pending_mutex_);
			pending_changed_.wait(lock, [this] { return finishing_ || !pending_.empty(); });
			if (finishing_)
				return;
			event = std::move(pending_.front());
			pending_.pop_front();
		}
		// Only this thread writes, so it alone waits when the client is slow.
		if (!stream_->Write(event)) {
			request_close();
			return;
		}
	}
}


void Session::request_close()
{
	grpc::ServerContext* context = nullptr;
	{
		std::lock_guard lock(context_mutex_);
		if (closed_.exchange(true))
			return;
		context = context_;
		context_ = nullptr;
	}

	if (context != nullptr)
		context->TryCancel();
}


void Session::finish()
{
	// Cancelling wakes a writer that waits in Write for a client that does not read.
	request_close();
	{
		std::lock_guard lock(pending_mutex_);
		finishing_ = true;
		pending_.clear();
	}
	pending_changed_.notify_one();
	if (writer_.joinable() && writer_.get_id() != std::this_thread::get_id())
		writer_.join();
}


} // namespace will
