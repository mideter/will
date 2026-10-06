#pragma once

#include "consoleui.h"
#include "willclient.h"
#include "inbound_server_message_handler.h"

#include "infra/transport/messenger.pb.h"


namespace will {


/** Interactive chat session: loads history, then stdin send loop with inbound/closed handlers. */
class ChatSession {
public:
	ChatSession(WillClient& client, ConsoleUi& ui);

	void run();

private:
	template<typename Handler>
	void on_server_event(const v1::ServerEvent& event, Handler& handler) const;

	void loadHistory() const;

	/// An unborn body waits until it is born.
	void awaitBirth() const;

	WillClient& client_;
	ConsoleUi& ui_;
	mutable ShownRooms rooms_;
};


} // namespace will
