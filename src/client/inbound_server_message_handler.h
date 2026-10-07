#pragma once

#include "infra/transport/messenger.pb.h"

#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace will {


class ConsoleUi;
class WillClient;


/// The rooms last shown, in the order of their numbers, and whose abode they
/// are of (empty — one's own): `/room <number>` enters them.
class ShownRooms final {
public:
	/// The gaze turned to the abode of this host (empty — one's own) and into
	/// this room of it (empty — the abode itself), or its rooms were asked for:
	/// the rooms shown next are of it.
	void turned(std::string abode_of, std::string room = {});
	void shown(const v1::Rooms& rooms);

	/// The rooms shown are one's own, shown in one's upper room to be arranged.
	bool arranging() const;

	/// The room of this number and its abode; none if there is no such number.
	std::optional<std::pair<std::string, std::string>> numbered(std::string_view number) const;

private:
	mutable std::mutex mutex_;
	std::string looking_at_;
	std::string room_;
	std::string abode_of_;
	std::vector<std::string> names_;
};


class LoadingHistoryMessageHandler final {
public:
	LoadingHistoryMessageHandler(ConsoleUi& ui, ShownRooms& rooms);

	bool history_finished() const noexcept { return history_finished_; }

	void on(const v1::ServerEvent& event);

private:
	ConsoleUi& ui_;
	ShownRooms& rooms_;
	bool history_finished_ = false;
};


class ReceivingMessageHandler final {
public:
	ReceivingMessageHandler(const WillClient& client, ConsoleUi& ui, ShownRooms& rooms);

	void on(const v1::ServerEvent& event);

private:
	const WillClient& client_;
	ConsoleUi& ui_;
	ShownRooms& rooms_;
};


} // namespace will
