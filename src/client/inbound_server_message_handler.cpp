#include "inbound_server_message_handler.h"

#include "consoleui.h"
#include "willclient.h"

#include <stdexcept>
#include <string>


namespace will {


namespace {


void print_history_item(ConsoleUi& ui, const v1::HistoryItem& item)
{
	if (item.is_mine())
		ui.print_mine(item.body(), true);
	else
		ui.print_peer(item.name(), item.body(), true);
}


} // namespace


LoadingHistoryMessageHandler::LoadingHistoryMessageHandler(ConsoleUi& ui)
	: ui_(ui)
{}


void LoadingHistoryMessageHandler::on(const v1::ServerEvent& event)
{
	switch (event.event_case()) {
	case v1::ServerEvent::kHistoryItem:
		print_history_item(ui_, event.history_item());
		return;
	case v1::ServerEvent::kHistoryEnd:
		history_finished_ = true;
		return;
	default:
		throw std::runtime_error("Unexpected message while loading history");
	}
}


ReceivingMessageHandler::ReceivingMessageHandler(const WillClient& client, ConsoleUi& ui)
	: client_(client)
	, ui_(ui)
{}


void ReceivingMessageHandler::on(const v1::ServerEvent& event)
{
	switch (event.event_case()) {
	case v1::ServerEvent::kAuthOk:
	case v1::ServerEvent::kAuthRequired:
	case v1::ServerEvent::kHistoryEnd:
		return;
	case v1::ServerEvent::kReceiptAck:
		if (!client_.config().quiet_receipts)
			ui_.print_receipt();
		return;
	case v1::ServerEvent::kHistoryItem:
		print_history_item(ui_, event.history_item());
		return;
	case v1::ServerEvent::kChat:
		ui_.print_peer(event.chat().name(), event.chat().body());
		return;
	case v1::ServerEvent::kSupplicationOffer:
		ui_.print_status("Supplication from " + event.supplication_offer().suppliant_name()
						 + " — /accept " + event.supplication_offer().suppliant_name());
		return;
	case v1::ServerEvent::kTieFormed: {
		const auto& tie = event.tie_formed();
		const char* role = tie.as_novice() ? "novice" : "testator";
		ui_.print_status(std::string("Tie #") + std::to_string(tie.tie_id()) + " with "
						 + tie.counterpart_name() + " (you are " + role + ")");
		return;
	}
	case v1::ServerEvent::kDeedOffered: {
		const auto& deed = event.deed_offered();
		ui_.print_status("Deed #" + std::to_string(deed.deed_id()) + " from "
						 + deed.testator_name() + ": " + deed.body()
						 + " — /done " + std::to_string(deed.deed_id()));
		return;
	}
	case v1::ServerEvent::kDeedDone: {
		const auto& deed = event.deed_done();
		ui_.print_status("Deed #" + std::to_string(deed.deed_id()) + " done by "
						 + deed.novice_name());
		return;
	}
	case v1::ServerEvent::kProtocolNotice:
		ui_.print_status(event.protocol_notice().message());
		return;
	case v1::ServerEvent::EVENT_NOT_SET:
		break;
	}

	throw std::runtime_error("Will protocol: unhandled server message type");
}


} // namespace will
