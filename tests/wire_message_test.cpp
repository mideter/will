#include "wiremessage.h"
#include "wiremessage_codec.h"
#include "wiremessage_client.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"
#include "tcpframe.h"

#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>


namespace {


template<typename MessageType>
void assert_roundtrip(const MessageType& original)
{
	const auto encoded = will::WireMessageCodec::encode(original);
	const auto decoded = will::WireMessageCodec::decode(encoded);
	assert(decoded);
	const auto* roundtripped = dynamic_cast<const MessageType*>(decoded.get());
	assert(roundtripped);
	assert(*roundtripped == original);
}


template<typename MessageType>
bool decodes_as_client(const std::vector<char>& payload)
{
	const auto message = will::WireMessageCodec::decode_client(payload);
	return message && dynamic_cast<const MessageType*>(message.get()) != nullptr;
}


template<typename MessageType>
bool decodes_as_server(const std::vector<char>& payload)
{
	const auto message = will::WireMessageCodec::decode_server(payload);
	return message && dynamic_cast<const MessageType*>(message.get()) != nullptr;
}


} // namespace


int main()
{
	using namespace will;

	{
		const UserChatMessage chat{"hi"};
		const auto v = WireMessageCodec::encode(chat);
		assert(v.size() == 3);
		assert(static_cast<unsigned char>(v[0]) == static_cast<unsigned char>(WireMessage::Type::UserChat));
		assert(v[1] == 'h' && v[2] == 'i');
	}

	{
		const auto v = WireMessageCodec::encode(UserChatMessage{""});
		assert(v.size() == 1);
		assert(static_cast<unsigned char>(v[0]) == static_cast<unsigned char>(WireMessage::Type::UserChat));
	}

	{
		const auto a = WireMessageCodec::encode(ServerReceiptAckMessage{});
		assert(decodes_as_server<ServerReceiptAckMessage>(a));
		assert(a.size() == 1);
	}

	{
		const auto bind = WireMessageCodec::encode(BindTokenMessage{"device-token-32chars-long-enough"});
		assert(decodes_as_client<BindTokenMessage>(bind));
		const auto bind_message = WireMessageCodec::decode_client(bind);
		assert(bind_message);
		const auto* parsed_bind = dynamic_cast<const BindTokenMessage*>(bind_message.get());
		assert(parsed_bind);
		assert(parsed_bind->token() == "device-token-32chars-long-enough");
	}

	{
		const auto auth_required = WireMessageCodec::encode(AuthRequiredMessage{});
		assert(decodes_as_server<AuthRequiredMessage>(auth_required));
	}

	{
		const auto auth_ok = WireMessageCodec::encode(AuthOkMessage{});
		assert(decodes_as_server<AuthOkMessage>(auth_ok));
	}

	{
		const auto request = WireMessageCodec::encode(HistoryRequestMessage{50});
		assert(decodes_as_client<HistoryRequestMessage>(request));
		const auto request_message = WireMessageCodec::decode_client(request);
		assert(request_message);
		const auto* parsed_request = dynamic_cast<const HistoryRequestMessage*>(request_message.get());
		assert(parsed_request);
		assert(parsed_request->limit() == 50u);
	}

	{
		const auto item = WireMessageCodec::encode(HistoryItemMessage{42, true, "stored"});
		assert(decodes_as_server<HistoryItemMessage>(item));
		const auto item_message = WireMessageCodec::decode_server(item);
		assert(item_message);
		const auto* parsed = dynamic_cast<const HistoryItemMessage*>(item_message.get());
		assert(parsed);
		assert(parsed->message_id() == 42u);
		assert(parsed->is_mine());
		assert(parsed->body() == "stored");
	}

	{
		const auto end = WireMessageCodec::encode(HistoryEndMessage{});
		assert(decodes_as_server<HistoryEndMessage>(end));
	}

	{
		const auto v = WireMessageCodec::encode(UserChatMessage{"hi"});
		const std::string line = WireMessageCodec::format_for_log(v);
		assert(line.find("UserChat") != std::string::npos);
		assert(line.find("hi") != std::string::npos);
	}

	assert(WireMessageCodec::format_for_log(WireMessageCodec::encode(ServerReceiptAckMessage{})) == "ServerReceiptAck");
	assert(WireMessageCodec::format_for_log(WireMessageCodec::encode(HistoryEndMessage{})) == "HistoryEnd");
	assert(WireMessageCodec::format_for_log(WireMessageCodec::encode(AuthOkMessage{})) == "AuthOk");

	{
		const auto payload = WireMessageCodec::encode(UserChatMessage{"ping"});
		const auto frame = TcpFrame::encode(payload);
		assert(frame.size() == 4 + payload.size());
		std::array<unsigned char, 4> header{};
		std::memcpy(header.data(), frame.data(), header.size());
		const auto len = TcpFrame::read_u32_be(header);
		assert(len == payload.size());
		assert(frame[4] == payload[0]);
	}

	assert_roundtrip(UserChatMessage{"roundtrip"});
	assert_roundtrip(UserChatMessage{""});
	assert_roundtrip(ServerReceiptAckMessage{});
	assert_roundtrip(HistoryRequestMessage{50});
	assert_roundtrip(HistoryItemMessage{42, true, "stored"});
	assert_roundtrip(HistoryItemMessage{7, false, ""});
	assert_roundtrip(HistoryEndMessage{});
	assert_roundtrip(BindTokenMessage{"device-token-32chars-long-enough"});
	assert_roundtrip(AuthRequiredMessage{});
	assert_roundtrip(AuthOkMessage{});

	{
		const UserChatMessage chat{"bidirectional"};
		const auto encoded = WireMessageCodec::encode(chat);
		const auto client_decoded = WireMessageCodec::decode_client(encoded);
		const auto server_decoded = WireMessageCodec::decode_server(encoded);
		assert(client_decoded);
		assert(server_decoded);
		const auto* client_chat = dynamic_cast<const UserChatMessage*>(client_decoded.get());
		const auto* server_chat = dynamic_cast<const UserChatMessage*>(server_decoded.get());
		assert(client_chat);
		assert(server_chat);
		assert(*client_chat == chat);
		assert(*server_chat == chat);
	}

	return EXIT_SUCCESS;
}
