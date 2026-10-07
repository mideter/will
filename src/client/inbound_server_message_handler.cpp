#include "inbound_server_message_handler.h"

#include "consoleui.h"
#include "willclient.h"

#include <stdexcept>
#include <string>
#include <vector>


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


void print_rooms(ConsoleUi& ui, const v1::Rooms& rooms, const bool arranging)
{
	if (rooms.rooms().empty()) {
		ui.print_status("No room is open to you here.");
		return;
	}
	std::vector<std::vector<std::string>> rows;
	for (const v1::Room& room : rooms.rooms())
		rows.push_back({std::to_string(rows.size() + 1), room.name(), room.part() == v1::OUTER ? "outer" : "inner"});
	ui.print_list("Rooms", rows, arranging ? "/arrange inner|outer <number>" : "/room <number>");
}


void print_threshold(ConsoleUi& ui, const v1::Threshold& threshold)
{
	if (threshold.keeping()) {
		if (threshold.waiting().empty()) {
			ui.print_status("You keep the gates: no one waits.");
			return;
		}
		std::vector<std::vector<std::string>> rows;
		for (const std::string& name : threshold.waiting())
			rows.push_back({name});
		ui.print_list("You keep the gates; waiting", rows, "/admit <name>");
		return;
	}
	ui.print_status(threshold.open() ? "The gates are open: one who keeps them is here."
									 : "The gates are shut: no one who keeps them is here.");
}


void print_unborn(ConsoleUi& ui, const v1::Unborn& unborn)
{
	if (unborn.marks().empty()) {
		ui.print_status("No one awaits birth.");
		return;
	}
	std::vector<std::vector<std::string>> rows;
	for (const std::uint64_t mark : unborn.marks())
		rows.push_back({"#" + std::to_string(mark)});
	ui.print_list("Awaiting birth", rows, "/bear <mark>");
}


void print_outstanding(ConsoleUi& ui, const v1::Outstanding& outstanding)
{
	if (outstanding.behests().empty()) {
		ui.print_status("Nothing awaits you.");
		return;
	}
	std::vector<std::vector<std::string>> rows;
	for (const v1::OutstandingBehest& waiting : outstanding.behests())
		rows.push_back({waiting.room(), "#" + std::to_string(waiting.behest().id()), waiting.behest().body()});
	ui.print_list("Awaits you", rows, "/room <name>");
}


void print_word(ConsoleUi& ui, const v1::Word& word, const bool dim)
{
	const std::string number = "#" + std::to_string(word.id());
	switch (word.kind()) {
	case v1::Word::BEHEST:
		ui.print_status("Behest " + number + " from " + word.name() + ": " + word.body(),
						word.is_mine() ? std::string{} : "/done " + std::to_string(word.id()) + " [report]");
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


void ShownRooms::turned(std::string abode_of, std::string room)
{
	std::lock_guard lock(mutex_);
	looking_at_ = std::move(abode_of);
	room_ = std::move(room);
}


bool ShownRooms::arranging() const
{
	std::lock_guard lock(mutex_);
	return looking_at_.empty() && room_ == "Горница";
}


void ShownRooms::shown(const v1::Rooms& rooms)
{
	std::lock_guard lock(mutex_);
	abode_of_ = looking_at_;
	names_.clear();
	for (const v1::Room& room : rooms.rooms())
		names_.push_back(room.name());
}


std::optional<std::pair<std::string, std::string>> ShownRooms::numbered(const std::string_view number) const
{
	std::size_t index = 0;
	for (const char c : number) {
		if (c < '0' || c > '9')
			return std::nullopt;
		index = index * 10 + static_cast<std::size_t>(c - '0');
	}

	std::lock_guard lock(mutex_);
	if (number.empty() || index == 0 || index > names_.size())
		return std::nullopt;
	return std::pair{abode_of_, names_[index - 1]};
}


LoadingHistoryMessageHandler::LoadingHistoryMessageHandler(ConsoleUi& ui, ShownRooms& rooms)
	: ui_(ui)
	, rooms_(rooms)
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
		rooms_.shown(event.rooms());
		print_rooms(ui_, event.rooms(), rooms_.arranging());
		return;
	case v1::ServerEvent::kOutstanding:
		print_outstanding(ui_, event.outstanding());
		return;
	case v1::ServerEvent::kThreshold:
		print_threshold(ui_, event.threshold());
		return;
	case v1::ServerEvent::kUnborn:
		print_unborn(ui_, event.unborn());
		return;
	default:
		throw std::runtime_error("Unexpected message while loading history");
	}
}


ReceivingMessageHandler::ReceivingMessageHandler(const WillClient& client, ConsoleUi& ui, ShownRooms& rooms)
	: client_(client)
	, ui_(ui)
	, rooms_(rooms)
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
		ui_.print_notice("Supplication from " + event.supplication_offer().suppliant_name(),
						 "/accept " + event.supplication_offer().suppliant_name());
		return;
	case v1::ServerEvent::kTieFormed: {
		const auto& tie = event.tie_formed();
		const char* role = tie.as_novice() ? "novice" : "testator";
		ui_.print_notice(std::string("Tie #") + std::to_string(tie.tie_id()) + " with "
						 + tie.counterpart_name() + " (you are " + role + ")");
		return;
	}
	case v1::ServerEvent::kTurned: {
		const auto& turned = event.turned();
		rooms_.turned(turned.abode_of(), turned.room());
		const std::string abode = turned.abode_of().empty() ? "home" : "abode of " + turned.abode_of();
		ui_.print_header("── " + abode + (turned.room().empty() ? "" : " · " + turned.room()) + " ──");
		return;
	}
	case v1::ServerEvent::kRooms:
		rooms_.shown(event.rooms());
		print_rooms(ui_, event.rooms(), rooms_.arranging());
		return;
	case v1::ServerEvent::kOutstanding:
		print_outstanding(ui_, event.outstanding());
		return;
	case v1::ServerEvent::kThreshold:
		print_threshold(ui_, event.threshold());
		return;
	case v1::ServerEvent::kDwelling:
		ui_.print_notice("You dwell in the abode of " + event.dwelling().host_name() + " as "
							 + kind_name(event.dwelling().kind()),
						 "/visit " + event.dwelling().host_name());
		return;
	case v1::ServerEvent::kDwellings: {
		if (event.dwellings().dwellings().empty()) {
			ui_.print_status("You dwell in no other abode.");
			return;
		}
		std::vector<std::vector<std::string>> rows;
		for (const v1::Dwelling& dwelling : event.dwellings().dwellings())
			rows.push_back({dwelling.host_name(), kind_name(dwelling.kind())});
		ui_.print_list("You dwell with", rows, "/visit <name>");
		return;
	}
	case v1::ServerEvent::kSupplications: {
		if (event.supplications().suppliant_names().empty()) {
			ui_.print_status("No supplication awaits you.");
			return;
		}
		std::vector<std::vector<std::string>> rows;
		for (const std::string& name : event.supplications().suppliant_names())
			rows.push_back({name});
		ui_.print_list("Supplications", rows, "/accept <name> or /reject <name>");
		return;
	}
	case v1::ServerEvent::kDwellers: {
		if (event.dwellers().dwellers().empty()) {
			ui_.print_status("No one dwells in your abode.");
			return;
		}
		std::vector<std::vector<std::string>> rows;
		for (const v1::Dweller& dweller : event.dwellers().dwellers())
			rows.push_back({dweller.name(), kind_name(dweller.kind())});
		ui_.print_list("Dwellers", rows, "/regard <name> <kind>");
		return;
	}
	case v1::ServerEvent::kUnborn:
		print_unborn(ui_, event.unborn());
		return;
	case v1::ServerEvent::kLineage: {
		if (event.lineage().descents().empty()) {
			ui_.print_status("No one is your child by spirit.");
			return;
		}
		std::vector<std::vector<std::string>> rows;
		for (const v1::Descent& descent : event.lineage().descents())
			rows.push_back({descent.name(), "child of " + descent.father_name()});
		ui_.print_list("Your line", rows);
		return;
	}
	case v1::ServerEvent::kStirred:
		ui_.print_notice("New word from " + event.stirred().author_name() + " in " + event.stirred().room(),
						 "/room " + event.stirred().room());
		return;
	case v1::ServerEvent::kProtocolNotice:
		ui_.print_notice(event.protocol_notice().message());
		return;
	case v1::ServerEvent::EVENT_NOT_SET:
		break;
	}

	throw std::runtime_error("Will protocol: unhandled server message type");
}


} // namespace will
