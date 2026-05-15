#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "serveraddress.h"
#include "sockethandle.h"


namespace will {


/** Server → client: single-byte {@link WillMessage::ServerReceiptAck} frame. */
struct ServerReceiptAck {};


/** Peer chat text (UTF-8) after stripping {@link WillMessage::UserChat} prefix. */
using InboundMessage = std::variant<ServerReceiptAck, std::string>;


// TCP: TcpFrame; payload is typed Will message (see willmessage.h).
class MessengerClient {
public:
	MessengerClient();

	void connect(ServerAddress server);

	/** Sends {@code UserChat} with UTF-8 body. */
	void send(std::string_view utf8_chat_body) const;

	/** std::nullopt = peer closed before next frame header; otherwise typed inbound message. */
	std::optional<InboundMessage> receiveMessage() const;

	void shutdown() const;

private:
	SocketHandle socket_;
};


} // namespace will
