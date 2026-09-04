#include "consoleui.h"
#include "inbound_server_message_handler.h"
#include "willclient.h"

#include "infra/transport/messenger.pb.h"

#include <cassert>
#include <cstdlib>
#include <exception>
#include <iostream>


/** Linked only because ReceivingMessageHandler shares a translation unit. */
const will::ClientConfig& will::WillClient::config() const noexcept
{
	static const ClientConfig config;
	return config;
}


namespace {


will::v1::ServerEvent make_chat(const char* name, const char* body)
{
	will::v1::ServerEvent event;
	auto* chat = event.mutable_chat();
	chat->set_name(name);
	chat->set_body(body);
	return event;
}


will::v1::ServerEvent make_history_item(const char* body, const bool is_mine)
{
	will::v1::ServerEvent event;
	auto* item = event.mutable_history_item();
	item->set_message_id(1);
	item->set_is_mine(is_mine);
	item->set_name("peername");
	item->set_body(body);
	return event;
}


will::v1::ServerEvent make_history_end()
{
	will::v1::ServerEvent event;
	event.mutable_history_end();
	return event;
}


/** Live chat must not abort history loading (deferred until HistoryEnd is fine). */
void test_chat_during_history_does_not_throw()
{
	will::ConsoleUi ui(will::ColorMode::Never);
	will::LoadingHistoryMessageHandler handler(ui);

	handler.on(make_history_item("older", false));
	assert(!handler.history_finished());

	handler.on(make_chat("peername", "live-while-loading"));
	assert(!handler.history_finished());

	handler.on(make_history_end());
	assert(handler.history_finished());
}


} // namespace


int main()
{
	try {
		test_chat_during_history_does_not_throw();
	}
	catch (const std::exception& e) {
		std::cerr << "loading_history_handler_test failed: " << e.what() << '\n';
		return EXIT_FAILURE;
	}

	std::cout << "loading_history_handler_test passed\n";
	return EXIT_SUCCESS;
}
