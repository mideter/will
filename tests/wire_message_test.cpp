#include "wiremessage.h"
#include "willprotocol.h"

#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>


int main()
{
	using namespace will;

	{
		const auto v = WireMessage::encode_user_chat("hi");
		assert(v.size() == 3);
		assert(static_cast<unsigned char>(v[0]) == WireMessage::UserChat);
		assert(v[1] == 'h' && v[2] == 'i');
	}

	{
		const auto v = WireMessage::encode_user_chat("");
		assert(v.size() == 1);
		assert(static_cast<unsigned char>(v[0]) == WireMessage::UserChat);
	}

	{
		const auto a = WireMessage::encode_server_receipt_ack();
		assert(WireMessage::is_server_receipt_ack(a));
		assert(a.size() == 1);
	}

	assert(!WireMessage::is_valid_client_to_server_payload({}));
	assert(WireMessage::is_valid_client_to_server_payload({'\1'}));
	assert(WireMessage::is_valid_client_to_server_payload({'\1', 'x'}));
	assert(!WireMessage::is_valid_client_to_server_payload({'\2'}));
	assert(!WireMessage::is_valid_client_to_server_payload({'\0'}));

	{
		const auto login = WireMessage::encode_login_request("alice", "secret");
		assert(WireMessage::is_login_request(login));
		assert(WireMessage::is_valid_client_to_server_payload(login));
		const auto parsed_login = WireMessage::parse_login_request(login);
		assert(parsed_login);
		assert(parsed_login->login == "alice");
		assert(parsed_login->password == "secret");
	}

	{
		const auto ok = WireMessage::encode_login_response_success("session-token");
		assert(WireMessage::is_login_response(ok));
		const auto parsed_ok = WireMessage::parse_login_response(ok);
		assert(parsed_ok);
		assert(parsed_ok->success);
		assert(parsed_ok->token == "session-token");
	}

	{
		const auto fail = WireMessage::encode_login_response_failure(WireMessage::LoginErrorInvalidCredentials);
		const auto parsed_fail = WireMessage::parse_login_response(fail);
		assert(parsed_fail);
		assert(!parsed_fail->success);
		assert(parsed_fail->error_code == WireMessage::LoginErrorInvalidCredentials);
	}

	{
		const auto bind = WireMessage::encode_bind_token("session-token");
		assert(WireMessage::is_bind_token(bind));
		assert(WireMessage::is_valid_client_to_server_payload(bind));
		assert(WireMessage::parse_bind_token(bind) == "session-token");
	}

	{
		const auto auth_required = WireMessage::encode_auth_required();
		assert(WireMessage::is_auth_required(auth_required));
	}

	{
		const auto request = WireMessage::encode_history_request(50);
		assert(WireMessage::is_history_request(request));
		assert(WireMessage::is_valid_client_to_server_payload(request));
		assert(WireMessage::parse_history_request_limit(request) == 50u);
	}

	{
		const auto item = WireMessage::encode_history_item(42, true, "stored");
		assert(WireMessage::is_history_item(item));
		const auto parsed = WireMessage::parse_history_item(item);
		assert(parsed);
		assert(parsed->message_id == 42u);
		assert(parsed->is_mine);
		assert(parsed->body == "stored");
	}

	{
		const auto end = WireMessage::encode_history_end();
		assert(WireMessage::is_history_end(end));
	}

	{
		const auto v = WireMessage::encode_user_chat("hi");
		const std::string line = WireMessage::format_payload_for_log(v);
		assert(line.find("UserChat") != std::string::npos);
		assert(line.find("hi") != std::string::npos);
	}

	assert(WireMessage::format_payload_for_log(WireMessage::encode_server_receipt_ack()) == "ServerReceiptAck");
	assert(WireMessage::format_payload_for_log(WireMessage::encode_history_end()) == "HistoryEnd");

	{
		const auto payload = WireMessage::encode_user_chat("ping");
		const auto frame = TcpFrame::encode(payload);
		assert(frame.size() == 4 + payload.size());
		std::array<unsigned char, 4> header{};
		std::memcpy(header.data(), frame.data(), header.size());
		const auto len = TcpFrame::read_u32_be(header);
		assert(len == payload.size());
		assert(frame[4] == payload[0]);
	}

	{
		const UserChat chat{"roundtrip"};
		const auto encoded = encode(chat);
		const auto decoded = decode(encoded);
		assert(decoded && std::get<UserChat>(*decoded) == chat);
	}

	{
		const ServerReceiptAck ack{};
		const auto encoded = encode(ack);
		const auto decoded = decode(encoded);
		assert(decoded && std::get<ServerReceiptAck>(*decoded) == ack);
	}

	{
		const HistoryRequest request{50};
		const auto encoded = encode(request);
		const auto decoded = decode(encoded);
		assert(decoded && std::get<HistoryRequest>(*decoded) == request);
	}

	{
		const HistoryItemPayload item{42, true, "stored"};
		const auto encoded = encode(item);
		const auto decoded = decode(encoded);
		assert(decoded && std::get<HistoryItemPayload>(*decoded) == item);
	}

	{
		const HistoryEnd end{};
		const auto encoded = encode(end);
		const auto decoded = decode(encoded);
		assert(decoded && std::get<HistoryEnd>(*decoded) == end);
	}

	{
		const LoginRequestPayload login{"alice", "secret"};
		const auto encoded = encode(login);
		const auto decoded = decode(encoded);
		assert(decoded && std::get<LoginRequestPayload>(*decoded) == login);
	}

	{
		const LoginResponsePayload ok_response{true, "session-token", 0};
		const auto encoded = encode(ok_response);
		const auto decoded = decode(encoded);
		assert(decoded && std::get<LoginResponsePayload>(*decoded) == ok_response);
	}

	{
		const LoginResponsePayload fail_response{false, "", WireMessage::LoginErrorInvalidCredentials};
		const auto encoded = encode(fail_response);
		const auto decoded = decode(encoded);
		assert(decoded && std::get<LoginResponsePayload>(*decoded) == fail_response);
	}

	{
		const BindToken bind{"session-token"};
		const auto encoded = encode(bind);
		const auto decoded = decode(encoded);
		assert(decoded && std::get<BindToken>(*decoded) == bind);
	}

	{
		const AuthRequired auth_required{};
		const auto encoded = encode(auth_required);
		const auto decoded = decode(encoded);
		assert(decoded && std::get<AuthRequired>(*decoded) == auth_required);
	}

	return EXIT_SUCCESS;
}
