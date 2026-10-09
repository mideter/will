#include "chatsession.h"

#include "inbound_server_message_handler.h"

#include <atomic>
#include <cstdio>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <poll.h>
#include <unistd.h>


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
	case v1::ServerEvent::kThreshold:
	case v1::ServerEvent::kUnborn:
	case v1::ServerEvent::kLineage:
	case v1::ServerEvent::kUnderway:
	case v1::ServerEvent::kExerted:
		return true;
	default:
		return false;
	}
}


std::string_view trimmed(std::string_view text)
{
	while (!text.empty() && text.front() == ' ')
		text.remove_prefix(1);
	while (!text.empty() && text.back() == ' ')
		text.remove_suffix(1);
	return text;
}


/// Seconds as typed: «90» or «1:30». None if it is not a span.
std::optional<std::uint32_t> seconds_of(std::string_view text)
{
	const auto colon = text.find(':');
	try {
		std::size_t used = 0;
		const std::string first{text.substr(0, colon)};
		std::uint32_t seconds = static_cast<std::uint32_t>(std::stoul(first, &used));
		if (used != first.size())
			return std::nullopt;
		if (colon != std::string_view::npos) {
			const std::string second{text.substr(colon + 1)};
			const unsigned long within = std::stoul(second, &used);
			if (used != second.size() || second.size() != 2 || within > 59)
				return std::nullopt;
			seconds = seconds * 60 + static_cast<std::uint32_t>(within);
		}
		return seconds;
	} catch (const std::exception&) {
		return std::nullopt;
	}
}


/// An approach as typed: «60x10», «62.5x8», «0x8» (one's own weight), with the rest
/// before it: «1:30/60x10» or «90/60x10». None if it is not one.
std::optional<v1::Approach> approach_of(std::string_view token)
{
	std::uint32_t rest = 0;
	if (const auto slash = token.find('/'); slash != std::string_view::npos) {
		const auto seconds = seconds_of(token.substr(0, slash));
		if (!seconds)
			return std::nullopt;
		rest = *seconds;
		token = token.substr(slash + 1);
	}
	const auto x = token.find_first_of("xх×");
	if (x == std::string_view::npos || x == 0)
		return std::nullopt;
	std::string kilograms{token.substr(0, x)};
	for (char& c : kilograms) {
		if (c == ',')
			c = '.';
	}
	const std::size_t after = token.find_first_not_of("xх×", x);  // «х» and «×» take more than one byte
	if (after == std::string_view::npos)
		return std::nullopt;
	try {
		std::size_t used = 0;
		const double weight = std::stod(kilograms, &used);
		const std::string repetitions{token.substr(after)};
		std::size_t counted = 0;
		const unsigned long reps = std::stoul(repetitions, &counted);
		if (used != kilograms.size() || counted != repetitions.size() || weight < 0)
			return std::nullopt;
		v1::Approach approach;
		approach.set_weight_grams(static_cast<std::uint32_t>(weight * 1000 + 0.5));
		approach.set_repetitions(static_cast<std::uint32_t>(reps));
		approach.set_rest_seconds(rest);
		return approach;
	} catch (const std::exception&) {
		return std::nullopt;
	}
}


/// Exercises as typed, separated by «|»: a name, then its approaches. None if one has no approach.
std::optional<std::vector<v1::Exercise>> exercises_of(std::string_view text)
{
	std::vector<v1::Exercise> exercises;
	while (!text.empty()) {
		const auto bar = text.find('|');
		const std::string_view part = trimmed(text.substr(0, bar));
		text = bar == std::string_view::npos ? std::string_view{} : text.substr(bar + 1);
		if (part.empty())
			continue;

		// The approaches are the last words; what is before them is the name.
		std::vector<std::string_view> words;
		for (std::string_view rest = part; !rest.empty();) {
			const auto space = rest.find(' ');
			if (const std::string_view word = rest.substr(0, space); !word.empty())
				words.push_back(word);
			rest = space == std::string_view::npos ? std::string_view{} : rest.substr(space + 1);
		}
		std::size_t first = words.size();
		while (first > 0 && approach_of(words[first - 1]))
			--first;
		if (first == words.size() || first == 0)
			return std::nullopt;

		v1::Exercise exercise;
		std::string name;
		for (std::size_t i = 0; i < first; ++i)
			name += (i ? " " : "") + std::string{words[i]};
		exercise.set_name(name);
		for (std::size_t i = first; i < words.size(); ++i)
			*exercise.add_approaches() = *approach_of(words[i]);
		exercises.push_back(std::move(exercise));
	}
	return exercises;
}


/// The commands, a group to a line.
void print_help(ConsoleUi& ui)
{
	ui.print_commands("Rooms", "/rooms [name]  /room <number|name>  /home  /visit <name> [room]");
	ui.print_commands("Gates", "/gates <name>  /admit <name>");
	ui.print_commands("Dwellers", "/dwellers  /regard <name> acquaintance|neighbour|friend  /dwellings");
	ui.print_commands("Arranging", "/arrange inner|outer <number|room>  (in the upper room, as /regard)");
	ui.print_commands("Obedience", "/ask <name>  /accept <name>  /reject <name>  /supplications  /done <number> [report]");
	ui.print_commands("Training", "/train [title] | <exercise> 60x10 1:30/70x8 2:00/80x6 | …  (1:30/ — rest before)");
	ui.print_commands("", "/begin <training> <exercise> <approach>  /finish <training> 60x10  /done <training> [report]");
	ui.print_commands("Birth", "/birth (your gates: the unborn)  /bear <mark>");
	ui.print_status("Type text in a room to write there.", "Ctrl+D to exit");
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

	if (cmd == "help") {
		print_help(ui);
		return true;
	}
	if (cmd == "ask") {
		if (args.empty()) {
			ui.print_notice("usage: /ask <name>");
			return true;
		}
		client.ask(args);
		return true;
	}
	if (cmd == "accept") {
		if (args.empty()) {
			ui.print_notice("usage: /accept <name>");
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
		// After «|», what was done in a training: each exercise with its approaches.
		const auto bar = args.find('|');
		const std::string_view head = trimmed(args.substr(0, bar));
		const auto sp = head.find(' ');
		const std::string_view number = head.substr(0, sp);
		const std::string_view report = sp == std::string_view::npos ? std::string_view{} : trimmed(head.substr(sp + 1));
		std::vector<v1::Exercise> performed;
		if (bar != std::string_view::npos) {
			const auto done = exercises_of(args.substr(bar + 1));
			if (!done) {
				ui.print_notice("usage: /done <number> [report] | <exercise> 60x10 70x8 | …");
				return true;
			}
			performed = *done;
		}
		try {
			client.fulfil(std::stoull(std::string{number}), report, performed);
		} catch (const std::invalid_argument&) {
			ui.print_notice("usage: /done <number> [report] [| <exercise> 60x10 70x8 | …]");
		} catch (const std::out_of_range&) {
			ui.print_notice("usage: /done <number> [report] [| <exercise> 60x10 70x8 | …]");
		}
		return true;
	}
	if (cmd == "begin") {
		// Numbers as one reads them, from one.
		unsigned long n = 0, e = 0, a = 0;
		if (std::sscanf(std::string{args}.c_str(), "%lu %lu %lu", &n, &e, &a) != 3 || e == 0 || a == 0) {
			ui.print_notice("usage: /begin <training> <exercise> <approach>");
			return true;
		}
		client.begin_approach(n, static_cast<std::uint32_t>(e - 1), static_cast<std::uint32_t>(a - 1));
		return true;
	}
	if (cmd == "finish") {
		const auto sp = args.find(' ');
		const auto approach = sp == std::string_view::npos ? std::nullopt : approach_of(trimmed(args.substr(sp + 1)));
		if (!approach) {
			ui.print_notice("usage: /finish <training> <weight>x<repetitions>");
			return true;
		}
		try {
			client.finish_approach(std::stoull(std::string{args.substr(0, sp)}), approach->weight_grams(),
								   approach->repetitions());
		} catch (const std::exception&) {
			ui.print_notice("usage: /finish <training> <weight>x<repetitions>");
		}
		return true;
	}
	if (cmd == "train") {
		const auto bar = args.find('|');
		const auto exercises = bar == std::string_view::npos ? std::nullopt : exercises_of(args.substr(bar + 1));
		if (!exercises || exercises->empty()) {
			ui.print_notice("usage: /train [title] | <exercise> 60x10 1:30/70x8 | <exercise> 0x8 …");
			return true;
		}
		client.train(trimmed(args.substr(0, bar)), *exercises);
		return true;
	}

	if (cmd == "visit") {
		if (args.empty()) {
			ui.print_notice("usage: /visit <name> [room]");
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
			ui.print_notice("usage: /room <number|name>");
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
			ui.print_notice("usage: /arrange inner|outer <number|room>");
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
			ui.print_notice("usage: /admit <name>");
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
			ui.print_notice("usage: /regard <name> acquaintance|neighbour|friend");
			return true;
		}
		if (kind == "acquaintance")
			client.regard(name, v1::ACQUAINTANCE);
		else if (kind == "neighbour")
			client.regard(name, v1::NEIGHBOUR);
		else if (kind == "friend")
			client.regard(name, v1::FRIEND);
		else
			ui.print_notice("usage: /regard <name> acquaintance|neighbour|friend");
		return true;
	}
	if (cmd == "dwellers") {
		client.list_dwellers();
		return true;
	}
	if (cmd == "gates") {
		if (args.empty()) {
			ui.print_notice("usage: /gates <name>");
			return true;
		}
		client.visit(args, "Врата");
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
	if (cmd == "bear") {
		try {
			client.bear(std::stoull(std::string{args.starts_with('#') ? args.substr(1) : args}));
		} catch (const std::invalid_argument&) {
			ui.print_notice("usage: /bear <mark>");
		} catch (const std::out_of_range&) {
			ui.print_notice("usage: /bear <mark>");
		}
		return true;
	}
	if (cmd == "birth") {
		// The unborn are seen at one's own gates.
		client.visit({}, "Врата");
		return true;
	}
	if (cmd == "reject") {
		if (args.empty()) {
			ui.print_notice("usage: /reject <name>");
			return true;
		}
		client.reject(args);
		return true;
	}

	ui.print_notice("unknown command", "/help lists them");
	return true;
}


} // namespace


ChatSession::ChatSession(WillClient& client, ConsoleUi& ui)
	: client_(client)
	, ui_(ui)
{}


void ChatSession::run()
{
	if (!awaitBirth()) {
		client_.shutdown();
		return;
	}
	loadHistory();

	std::atomic<bool> disconnected{false};

	// The main thread is blocked reading input, so the reader thread says it at once.
	client_.set_closed_handler([this, &disconnected] {
		if (!disconnected.exchange(true))
			ui_.print_notice("Disconnected from chat. Press Enter to exit.");
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
	ui_.print_status("Type text in a room to write there.", "/help for commands, Ctrl+D to exit");
	ui_.set_live_prompt(true);
	ui_.print_prompt();

	std::string line;
	while (!disconnected.load() && std::getline(std::cin, line)) {
		ui_.line_entered();
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


bool ChatSession::awaitBirth() const
{
	if (!client_.mark())
		return true;

	ui_.print_header("You are not yet born.");
	ui_.print_status("Your body is seen as #" + std::to_string(*client_.mark())
						 + " at the gates: wait until someone bears you.",
					 "Ctrl+D to leave");

	std::mutex mutex;
	std::optional<std::string> name;
	bool disconnected = false;

	client_.set_closed_handler([&] {
		std::lock_guard lock(mutex);
		disconnected = true;
	});
	client_.set_inbound_handler([&](const v1::ServerEvent& event) {
		if (!event.has_auth_ok() || event.auth_ok().unborn())
			return;
		std::lock_guard lock(mutex);
		name = event.auth_ok().name();
	});

	// Waiting, one still reads what is typed: each line is answered at once, so
	// nothing is kept to be done after the birth; Ctrl+D leaves. The input is
	// looked at in short turns, that the birth is not held up by a read.
	bool left = false;
	ui_.set_live_prompt(true);
	ui_.print_prompt();
	while (true) {
		{
			std::lock_guard lock(mutex);
			if (name || disconnected)
				break;
		}

		if (std::cin.rdbuf()->in_avail() <= 0) {
			pollfd input{STDIN_FILENO, POLLIN, 0};
			if (::poll(&input, 1, 200) <= 0)
				continue;
		}

		std::string line;
		if (!std::getline(std::cin, line)) {
			left = true;
			break;
		}
		ui_.line_entered();
		if (!line.empty())
			ui_.print_notice("You are not yet born: nothing is done until someone bears you.");
		ui_.print_prompt();
	}
	ui_.set_live_prompt(false);

	client_.set_closed_handler(nullptr);
	client_.set_inbound_handler(nullptr);

	if (left)
		return false;

	std::lock_guard lock(mutex);
	if (!name)
		throw std::runtime_error("Disconnected while awaiting birth");

	client_.born(*name);
	ui_.print_header("You are born.");
	return true;
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
