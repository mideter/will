#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "consoleui.h"
#include "inbound_server_message_handler.h"
#include "willclient.h"

#include "infra/transport/messenger.pb.h"

#include <stdexcept>


/** Linked only because ReceivingMessageHandler shares a translation unit. */
const will::ClientConfig& will::WillClient::config() const noexcept
{
	static const ClientConfig config;
	return config;
}


namespace {


will::v1::ServerEvent make_tie_formed()
{
	will::v1::ServerEvent event;
	auto* tie = event.mutable_tie_formed();
	tie->set_tie_id(3);
	tie->set_counterpart_name("peername");
	return event;
}


will::v1::ServerEvent make_history_item(const char* body, const bool is_mine)
{
	will::v1::ServerEvent event;
	auto* item = event.mutable_word();
	item->set_id(1);
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


} // namespace


TEST_CASE("history load accepts items then HistoryEnd")
{
	will::ConsoleUi ui(will::ColorMode::Never);
	will::LoadingHistoryMessageHandler handler(ui);

	handler.on(make_history_item("older", false));
	CHECK_FALSE(handler.history_finished());

	handler.on(make_history_end());
	CHECK(handler.history_finished());
}


TEST_CASE("a word placed while history loads is told with it; other events are rejected")
{
	will::ConsoleUi ui(will::ColorMode::Never);
	will::LoadingHistoryMessageHandler handler(ui);

	handler.on(make_history_item("older", false));
	handler.on(make_history_item("live-while-loading", false));
	CHECK_FALSE(handler.history_finished());

	CHECK_THROWS_AS(handler.on(make_tie_formed()), std::runtime_error);
	CHECK_FALSE(handler.history_finished());
}
