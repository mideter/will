#include "wiremessage.h"
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
	const auto encoded = will::encode(original);
	const auto decoded = will::decode_message(encoded);
	assert(decoded);
	const auto* roundtripped = dynamic_cast<const MessageType*>(decoded.get());
	assert(roundtripped);
	assert(*roundtripped == original);
}


template<typename MessageType>
bool decodes_as_client(const std::vector<char>& payload)
{
	const auto message = will::decode_client_message(payload);
	return message && dynamic_cast<const MessageType*>(message.get()) != nullptr;
}


template<typename MessageType>
bool decodes_as_server(const std::vector<char>& payload)
{
	const auto message = will::decode_server_message(payload);
	return message && dynamic_cast<const MessageType*>(message.get()) != nullptr;
}


} // namespace


int main()
{
	using namespace will;

	{
		const UserChatMessage chat{"hi"};
		const auto v = encode(chat);
		assert(v.size() == 3);
		assert(static_cast<unsigned char>(v[0]) == static_cast<unsigned char>(WireMessage::Type::UserChat));
		assert(v[1] == 'h' && v[2] == 'i');
	}

	{
		const auto v = encode(UserChatMessage{""});
		assert(v.size() == 1);
		assert(static_cast<unsigned char>(v[0]) == static_cast<unsigned char>(WireMessage::Type::UserChat));
	}

	{
		const auto a = encode(ServerReceiptAckMessage{});
		assert(decodes_as_server<ServerReceiptAckMessage>(a));
		assert(a.size() == 1);
	}

	{
		const auto login = encode(LoginRequestMessage{"alice", "secret"});
		assert(decodes_as_client<LoginRequestMessage>(login));
		const auto login_message = decode_client_message(login);
		assert(login_message);
		const auto* parsed_login = dynamic_cast<const LoginRequestMessage*>(login_message.get());
		assert(parsed_login);
		assert(parsed_login->login() == "alice");
		assert(parsed_login->password() == "secret");
	}

	{
		const auto ok = encode(LoginResponseMessage{true, "session-token", 0});
		assert(decodes_as_server<LoginResponseMessage>(ok));
		const auto ok_message = decode_server_message(ok);
		assert(ok_message);
		const auto* parsed_ok = dynamic_cast<const LoginResponseMessage*>(ok_message.get());
		assert(parsed_ok);
		assert(parsed_ok->success());
		assert(parsed_ok->token() == "session-token");
	}

	{
		const auto fail = encode(LoginResponseMessage{
		    false, "", static_cast<std::uint8_t>(LoginError::InvalidCredentials)});
		const auto fail_message = decode_server_message(fail);
		assert(fail_message);
		const auto* parsed_fail = dynamic_cast<const LoginResponseMessage*>(fail_message.get());
		assert(parsed_fail);
		assert(!parsed_fail->success());
		assert(parsed_fail->error_code() == static_cast<std::uint8_t>(LoginError::InvalidCredentials));
	}

	{
		const auto bind = encode(BindTokenMessage{"session-token"});
		assert(decodes_as_client<BindTokenMessage>(bind));
		const auto bind_message = decode_client_message(bind);
		assert(bind_message);
		const auto* parsed_bind = dynamic_cast<const BindTokenMessage*>(bind_message.get());
		assert(parsed_bind);
		assert(parsed_bind->token() == "session-token");
	}

	{
		const auto auth_required = encode(AuthRequiredMessage{});
		assert(decodes_as_server<AuthRequiredMessage>(auth_required));
	}

	{
		const auto request = encode(HistoryRequestMessage{50});
		assert(decodes_as_client<HistoryRequestMessage>(request));
		const auto request_message = decode_client_message(request);
		assert(request_message);
		const auto* parsed_request = dynamic_cast<const HistoryRequestMessage*>(request_message.get());
		assert(parsed_request);
		assert(parsed_request->limit() == 50u);
	}

	{
		const auto item = encode(HistoryItemMessage{42, true, "stored"});
		assert(decodes_as_server<HistoryItemMessage>(item));
		const auto item_message = decode_server_message(item);
		assert(item_message);
		const auto* parsed = dynamic_cast<const HistoryItemMessage*>(item_message.get());
		assert(parsed);
		assert(parsed->message_id() == 42u);
		assert(parsed->is_mine());
		assert(parsed->body() == "stored");
	}

	{
		const auto end = encode(HistoryEndMessage{});
		assert(decodes_as_server<HistoryEndMessage>(end));
	}

	{
		const auto v = encode(UserChatMessage{"hi"});
		const std::string line = format_for_log(v);
		assert(line.find("UserChat") != std::string::npos);
		assert(line.find("hi") != std::string::npos);
	}

	assert(format_for_log(encode(ServerReceiptAckMessage{})) == "ServerReceiptAck");
	assert(format_for_log(encode(HistoryEndMessage{})) == "HistoryEnd");

	{
		const auto payload = encode(UserChatMessage{"ping"});
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
	assert_roundtrip(LoginRequestMessage{"alice", "secret"});
	assert_roundtrip(LoginResponseMessage{true, "session-token", 0});
	assert_roundtrip(LoginResponseMessage{
	    false, "", static_cast<std::uint8_t>(LoginError::InvalidCredentials)});
	assert_roundtrip(LoginResponseMessage{
	    false, "", static_cast<std::uint8_t>(LoginError::ExpiredToken)});
	assert_roundtrip(BindTokenMessage{"session-token"});
	assert_roundtrip(AuthRequiredMessage{});

	{
		const UserChatMessage chat{"bidirectional"};
		const auto encoded = encode(chat);
		const auto client_decoded = decode_client_message(encoded);
		const auto server_decoded = decode_server_message(encoded);
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
