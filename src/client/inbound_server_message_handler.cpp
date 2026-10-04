#include "inbound_server_message_handler.h"

#include "consoleui.h"
#include "willclient.h"

#include <stdexcept>
#include <string>


namespace will {


namespace {


void print_word(ConsoleUi& ui, const v1::Word& word, const bool dim)
{
	const std::string number = "#" + std::to_string(word.id());
	switch (word.kind()) {
	case v1::Word::BEHEST:
		ui.print_status("Behest " + number + " from " + word.name() + ": " + word.body()
						+ (word.is_mine() ? std::string{} : " — /done " + std::to_string(word.id())));
		return;
	case v1::Word::DEED:
		ui.print_status("Deed " + number + " by " + word.name() + " fulfils #"
						+ std::to_string(word.behest_id()) + ": " + word.body());
		return;
	default:
		if (word.is_mine())
			ui.print_mine(word.body(), dim, false);
		else
			ui.print_peer(word.name(), word.body(), dim);
		return;
	}
}


} // namespace


LoadingHistoryMessageHandler::LoadingHistoryMessageHandler(ConsoleUi& ui)
	: ui_(ui)
{}


void LoadingHistoryMessageHandler::on(const v1::ServerEvent& event)
{
	switch (event.event_case()) {
	case v1::ServerEvent::kWord:
		print_word(ui_, event.word(), true);
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
	case v1::ServerEvent::kWord:
		print_word(ui_, event.word(), false);
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
	case v1::ServerEvent::kTurned:
		ui_.print_status(event.turned().tie_with().empty() ? "── home ──"
														   : "── tie with " + event.turned().tie_with() + " ──");
		return;
	case v1::ServerEvent::kStirred:
		ui_.print_status("New word in the tie with " + event.stirred().tie_with() + " — /tie "
						 + event.stirred().tie_with());
		return;
	case v1::ServerEvent::kProtocolNotice:
		ui_.print_status(event.protocol_notice().message());
		return;
	case v1::ServerEvent::EVENT_NOT_SET:
		break;
	}

	throw std::runtime_error("Will protocol: unhandled server message type");
}


} // namespace will
