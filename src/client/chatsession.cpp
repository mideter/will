#include "chatsession.h"

#include "inbound_server_message_handler.h"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>


namespace will {


namespace {


bool is_post_auth_server_event(const v1::ServerEvent& event) noexcept
{
	switch (event.event_case()) {
	case v1::ServerEvent::kReceiptAck:
	case v1::ServerEvent::kAuthRequired:
	case v1::ServerEvent::kWord:
	case v1::ServerEvent::kHistoryEnd:
	case v1::ServerEvent::kSupplicationOffer:
	case v1::ServerEvent::kTieFormed:
	case v1::ServerEvent::kTurned:
	case v1::ServerEvent::kStirred:
	case v1::ServerEvent::kProtocolNotice:
	case v1::ServerEvent::kDwelling:
	case v1::ServerEvent::kDwellers:
	case v1::ServerEvent::kRooms:
	case v1::ServerEvent::kOutstanding:
	case v1::ServerEvent::kDwellings:
	case v1::ServerEvent::kSupplications:
		return true;
	default:
		return false;
	}
}


bool handle_slash_command(WillClient& client, ConsoleUi& ui, ShownRooms& rooms, const std::string& line)
{
	if (line.empty() || line[0] != '/')
		return false;

	std::string_view rest{line};
	rest.remove_prefix(1);
	const auto space = rest.find(' ');
	const std::string_view cmd = space == std::string_view::npos ? rest : rest.substr(0, space);
	std::string_view args = space == std::string_view::npos ? std::string_view{} : rest.substr(space + 1);
	while (!args.empty() && args.front() == ' ')
		args.remove_prefix(1);

	if (cmd == "ask") {
		if (args.empty()) {
			ui.print_status("usage: /ask <name>");
			return true;
		}
		client.ask(args);
		return true;
	}
	if (cmd == "accept") {
		if (args.empty()) {
			ui.print_status("usage: /accept <name>");
			return true;
		}
		client.accept(args);
		return true;
	}
	if (cmd == "home") {
		client.visit({});
		return true;
	}
	if (cmd == "done") {
		const auto sp = args.find(' ');
		const std::string_view number = args.substr(0, sp);
		const std::string_view report = sp == std::string_view::npos ? std::string_view{} : args.substr(sp + 1);
		try {
			client.fulfil(std::stoull(std::string{number}), report);
		} catch (const std::invalid_argument&) {
			ui.print_status("usage: /done <number> [report]");
		} catch (const std::out_of_range&) {
			ui.print_status("usage: /done <number> [report]");
		}
		return true;
	}

	if (cmd == "visit") {
		if (args.empty()) {
			ui.print_status("usage: /visit <name> [room]");
			return true;
		}
		const auto sp = args.find(' ');
		const std::string_view host = args.substr(0, sp);
		const std::string_view room = sp == std::string_view::npos ? std::string_view{} : args.substr(sp + 1);
		client.visit(host, room);
		return true;
	}
	if (cmd == "room") {
		if (args.empty()) {
			ui.print_status("usage: /room <number|name>");
			return true;
		}
		// A number is of the rooms last shown, in the abode they were shown in.
		if (const auto numbered = rooms.numbered(args))
			client.visit(numbered->first, numbered->second);
		else
			client.visit({}, args);
		return true;
	}
	if (cmd == "arrange") {
		const auto sp = args.find(' ');
		const std::string_view part = args.substr(0, sp);
		std::string room{sp == std::string_view::npos ? std::string_view{} : args.substr(sp + 1)};
		if (room.empty() || (part != "inner" && part != "outer")) {
			ui.print_status("usage: /arrange inner|outer <number|room>");
			return true;
		}
		// A number is only of the rooms of one's own abode.
		if (const auto numbered = rooms.numbered(room); numbered && numbered->first.empty())
			room = numbered->second;
		client.arrange(room, part == "outer");
		return true;
	}
	if (cmd == "admit") {
		if (args.empty()) {
			ui.print_status("usage: /admit <name>");
			return true;
		}
		client.admit(args);
		return true;
	}
	if (cmd == "regard") {
		const auto sp = args.find(' ');
		const std::string_view name = args.substr(0, sp);
		const std::string_view kind = sp == std::string_view::npos ? std::string_view{} : args.substr(sp + 1);
		if (name.empty() || kind.empty()) {
			ui.print_status("usage: /regard <name> acquaintance|neighbour|friend");
			return true;
		}
		if (kind == "acquaintance")
			client.regard(name, v1::ACQUAINTANCE);
		else if (kind == "neighbour")
			client.regard(name, v1::NEIGHBOUR);
		else if (kind == "friend")
			client.regard(name, v1::FRIEND);
		else
			ui.print_status("usage: /regard <name> acquaintance|neighbour|friend");
		return true;
	}
	if (cmd == "dwellers") {
		client.list_dwellers();
		return true;
	}
	if (cmd == "rooms") {
		// The rooms shown next are of this abode: number them for it.
		rooms.turned(std::string{args});
		client.list_rooms(args);
		return true;
	}
	if (cmd == "dwellings") {
		client.list_dwellings();
		return true;
	}
	if (cmd == "supplications") {
		client.list_supplications();
		return true;
	}
	if (cmd == "reject") {
		if (args.empty()) {
			ui.print_status("usage: /reject <name>");
			return true;
		}
		client.reject(args);
		return true;
	}

	ui.print_status("unknown command; try /ask /accept /reject /supplications /home /done /rooms /room /arrange /admit "
					"/regard /dwellers /dwellings /visit");
	return true;
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

	// The main thread is blocked reading input, so the reader thread says it at once.
	client_.set_closed_handler([this, &disconnected] {
		if (!disconnected.exchange(true))
			ui_.print_status("Disconnected from chat. Press Enter to exit.");
	});
	client_.set_inbound_handler([this, &disconnected](const v1::ServerEvent& event) {
		if (disconnected.load())
			return;

		try {
			ReceivingMessageHandler handler{client_, ui_, rooms_};
			on_server_event(event, handler);
		}
		catch (const std::exception& e) {
			ui_.set_live_prompt(false);
			ui_.print_error(std::string("Receive error: ") + e.what());
			disconnected.store(true);
		}
	});

	ui_.print_status("Connected as " + client_.own_name() + ".");
	ui_.print_status("Chat: type text in a room. Obedience: /ask /accept /reject /supplications /done. "
					 "Rooms: /rooms /room /arrange. Dwellers: /admit /regard /dwellers /dwellings /visit. Ctrl+D to exit.");
	ui_.set_live_prompt(true);
	ui_.print_prompt();

	std::string line;
	while (!disconnected.load() && std::getline(std::cin, line)) {
		if (disconnected.load())
			break;

		if (line.empty()) {
			ui_.print_prompt();
			continue;
		}

		if (handle_slash_command(client_, ui_, rooms_, line)) {
			ui_.print_prompt();
			continue;
		}

		ui_.print_mine(line, false, !client_.config().quiet_receipts);
		client_.send(line);
	}

	ui_.set_live_prompt(false);

	// Leaving on our own is not a disconnect to report.
	client_.set_closed_handler(nullptr);
	client_.set_inbound_handler(nullptr);
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
			LoadingHistoryMessageHandler handler{ui_, rooms_};
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
