#include "chatsession.h"

#include "inbound_server_message_handler.h"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>


namespace will {


namespace {


bool is_post_auth_server_event(const v1::ServerEvent& event) noexcept
{
	switch (event.event_case()) {
	case v1::ServerEvent::kReceiptAck:
	case v1::ServerEvent::kAuthRequired:
	case v1::ServerEvent::kChat:
	case v1::ServerEvent::kHistoryItem:
	case v1::ServerEvent::kHistoryEnd:
		return true;
	default:
		return false;
	}
}


} // namespace


ChatSession::ChatSession(WillClient& client, ConsoleUi& ui)
	: client_(client)
	, ui_(ui)
{}


void ChatSession::run()
{
	loadHistory();

	std::atomic<bool> disconnected{false};

	client_.set_closed_handler([&disconnected] { disconnected.store(true); });
	client_.set_inbound_handler([this, &disconnected](const v1::ServerEvent& event) {
		if (disconnected.load())
			return;

		try {
			ReceivingMessageHandler handler{client_, ui_};
			on_server_event(event, handler);
		}
		catch (const std::exception& e) {
			ui_.set_live_prompt(false);
			ui_.print_error(std::string("Receive error: ") + e.what());
			disconnected.store(true);
		}
	});

	ui_.print_status("Connected to Will chat. Type messages and press Enter.");
	ui_.print_status("Press Ctrl+D to exit.");
	ui_.set_live_prompt(true);
	ui_.print_prompt();

	std::string line;
	while (!disconnected.load() && std::getline(std::cin, line)) {
		ui_.print_mine(line, false, !client_.config().quiet_receipts);
		client_.send(line);
	}

	ui_.set_live_prompt(false);

	if (disconnected.load())
		ui_.print_status("Disconnected from chat.");

	client_.shutdown();
}


void ChatSession::loadHistory() const
{
	if (client_.config().history_limit == 0)
		return;

	std::mutex mutex;
	std::condition_variable cv;
	bool finished = false;
	bool disconnected = false;
	std::string error_message;

	ui_.print_history_begin();

	client_.set_closed_handler([&] {
		std::lock_guard lock(mutex);
		if (!finished) {
			disconnected = true;
			cv.notify_one();
		}
	});

	client_.set_inbound_handler([&](const v1::ServerEvent& event) {
		try {
			LoadingHistoryMessageHandler handler{ui_};
			on_server_event(event, handler);

			std::lock_guard lock(mutex);
			if (handler.history_finished())
				finished = true;
			cv.notify_one();
		}
		catch (const std::exception& e) {
			std::lock_guard lock(mutex);
			error_message = e.what();
			disconnected = true;
			cv.notify_one();
		}
	});

	client_.requestHistory(client_.config().history_limit);

	std::unique_lock lock(mutex);
	cv.wait(lock, [&] { return finished || disconnected; });

	client_.set_closed_handler(nullptr);
	client_.set_inbound_handler(nullptr);

	if (!finished)
		throw std::runtime_error(disconnected && !error_message.empty() ? error_message
																		: "Disconnected while loading history");

	ui_.print_history_end();
}


template<typename Handler>
void ChatSession::on_server_event(const v1::ServerEvent& event, Handler& handler) const
{
	if (!is_post_auth_server_event(event))
		throw std::runtime_error("Will protocol: invalid server frame");

	handler.on(event);
}


template void ChatSession::on_server_event<LoadingHistoryMessageHandler>(const v1::ServerEvent&,
																		 LoadingHistoryMessageHandler&) const;
template void ChatSession::on_server_event<ReceivingMessageHandler>(const v1::ServerEvent&,
																	ReceivingMessageHandler&) const;


} // namespace will
