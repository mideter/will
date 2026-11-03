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


class ShownRooms final {
public:
	void turned(std::string abode_of, std::string room = {});
	void listing(std::string abode_of);
	void shown(const v1::Rooms& rooms);

	std::pair<std::string, std::string> standing() const;

	bool arranging() const;

	std::optional<std::pair<std::string, std::string>> numbered(std::string_view number) const;

private:
	mutable std::mutex mutex_;
	std::string looking_at_;
	std::string room_;
	std::pair<std::string, std::string> standing_;
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
