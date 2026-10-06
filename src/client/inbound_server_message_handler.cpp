#include "inbound_server_message_handler.h"

#include "consoleui.h"
#include "willclient.h"

#include <stdexcept>
#include <string>


namespace will {


namespace {


std::string kind_name(const v1::DwellerKind kind)
{
	switch (kind) {
	case v1::NEIGHBOUR:
		return "a neighbour";
	case v1::FRIEND:
		return "a friend";
	default:
		return "an acquaintance";
	}
}


void print_rooms(ConsoleUi& ui, const v1::Rooms& rooms)
{
	if (rooms.rooms().empty()) {
		ui.print_status("No room is open to you here.");
		return;
	}
	std::string told = "Rooms:";
	for (const v1::Room& room : rooms.rooms())
		told += " " + room.name() + (room.part() == v1::OUTER ? " (outer);" : " (inner);");
	ui.print_status(told + " — /room <name>");
}


void print_outstanding(ConsoleUi& ui, const v1::Outstanding& outstanding)
{
	if (outstanding.behests().empty()) {
		ui.print_status("Nothing awaits you.");
		return;
	}
	for (const v1::OutstandingBehest& waiting : outstanding.behests())
		ui.print_status("Awaits in " + waiting.room() + ": #" + std::to_string(waiting.behest().id()) + " "
						+ waiting.behest().body());
}


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
	case v1::ServerEvent::kRooms:
		print_rooms(ui_, event.rooms());
		return;
	case v1::ServerEvent::kOutstanding:
		print_outstanding(ui_, event.outstanding());
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
	case v1::ServerEvent::kTurned: {
		const auto& turned = event.turned();
		if (!turned.tie_with().empty()) {
			ui_.print_status("── tie with " + turned.tie_with() + " ──");
			return;
		}
		const std::string abode = turned.abode_of().empty() ? "home" : "abode of " + turned.abode_of();
		ui_.print_status("── " + abode + (turned.room().empty() ? "" : " · " + turned.room()) + " ──");
		return;
	}
	case v1::ServerEvent::kRooms:
		print_rooms(ui_, event.rooms());
		return;
	case v1::ServerEvent::kOutstanding:
		print_outstanding(ui_, event.outstanding());
		return;
	case v1::ServerEvent::kDwelling:
		ui_.print_status("You dwell in the abode of " + event.dwelling().host_name() + " as "
						 + kind_name(event.dwelling().kind()) + " — /visit " + event.dwelling().host_name());
		return;
	case v1::ServerEvent::kDwellers: {
		if (event.dwellers().dwellers().empty()) {
			ui_.print_status("No one dwells in your abode.");
			return;
		}
		std::string told = "Dwellers:";
		for (const v1::Dweller& dweller : event.dwellers().dwellers())
			told += " " + dweller.name() + " (" + kind_name(dweller.kind()) + ")";
		ui_.print_status(told);
		return;
	}
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
