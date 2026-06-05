#include "willmessage.h"
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
		const auto v = WillMessage::encode_user_chat("hi");
		assert(v.size() == 3);
		assert(static_cast<unsigned char>(v[0]) == WillMessage::UserChat);
		assert(v[1] == 'h' && v[2] == 'i');
	}

	{
		const auto v = WillMessage::encode_user_chat("");
		assert(v.size() == 1);
		assert(static_cast<unsigned char>(v[0]) == WillMessage::UserChat);
	}

	{
		const auto a = WillMessage::encode_server_receipt_ack();
		assert(WillMessage::is_server_receipt_ack(a));
		assert(a.size() == 1);
	}

	assert(!WillMessage::is_valid_client_to_server_payload({}));
	assert(WillMessage::is_valid_client_to_server_payload({'\1'}));
	assert(WillMessage::is_valid_client_to_server_payload({'\1', 'x'}));
	assert(!WillMessage::is_valid_client_to_server_payload({'\2'}));
	assert(!WillMessage::is_valid_client_to_server_payload({'\0'}));

	{
		const auto login = WillMessage::encode_login_request("alice", "secret");
		assert(WillMessage::is_login_request(login));
		assert(WillMessage::is_valid_client_to_server_payload(login));
		const auto parsed_login = WillMessage::parse_login_request(login);
		assert(parsed_login);
		assert(parsed_login->login == "alice");
		assert(parsed_login->password == "secret");
	}

	{
		const auto ok = WillMessage::encode_login_response_success("session-token");
		assert(WillMessage::is_login_response(ok));
		const auto parsed_ok = WillMessage::parse_login_response(ok);
		assert(parsed_ok);
		assert(parsed_ok->success);
		assert(parsed_ok->token == "session-token");
	}

	{
		const auto fail = WillMessage::encode_login_response_failure(WillMessage::LoginErrorInvalidCredentials);
		const auto parsed_fail = WillMessage::parse_login_response(fail);
		assert(parsed_fail);
		assert(!parsed_fail->success);
		assert(parsed_fail->error_code == WillMessage::LoginErrorInvalidCredentials);
	}

	{
		const auto bind = WillMessage::encode_bind_token("session-token");
		assert(WillMessage::is_bind_token(bind));
		assert(WillMessage::is_valid_client_to_server_payload(bind));
		assert(WillMessage::parse_bind_token(bind) == "session-token");
	}

	{
		const auto auth_required = WillMessage::encode_auth_required();
		assert(WillMessage::is_auth_required(auth_required));
	}

	{
		const auto request = WillMessage::encode_history_request(50);
		assert(WillMessage::is_history_request(request));
		assert(WillMessage::is_valid_client_to_server_payload(request));
		assert(WillMessage::parse_history_request_limit(request) == 50u);
	}

	{
		const auto item = WillMessage::encode_history_item(42, true, "stored");
		assert(WillMessage::is_history_item(item));
		const auto parsed = WillMessage::parse_history_item(item);
		assert(parsed);
		assert(parsed->message_id == 42u);
		assert(parsed->is_mine);
		assert(parsed->body == "stored");
	}

	{
		const auto end = WillMessage::encode_history_end();
		assert(WillMessage::is_history_end(end));
	}

	{
		const auto v = WillMessage::encode_user_chat("hi");
		const std::string line = WillMessage::format_payload_for_log(v);
		assert(line.find("UserChat") != std::string::npos);
		assert(line.find("hi") != std::string::npos);
	}

	assert(WillMessage::format_payload_for_log(WillMessage::encode_server_receipt_ack()) == "ServerReceiptAck");
	assert(WillMessage::format_payload_for_log(WillMessage::encode_history_end()) == "HistoryEnd");

	{
		const auto payload = WillMessage::encode_user_chat("ping");
		const auto frame = TcpFrame::encode(payload);
		assert(frame.size() == 4 + payload.size());
		std::array<unsigned char, 4> header{};
		std::memcpy(header.data(), frame.data(), header.size());
		const auto len = TcpFrame::read_u32_be(header);
		assert(len == payload.size());
		assert(frame[4] == payload[0]);
	}

	return EXIT_SUCCESS;
}
