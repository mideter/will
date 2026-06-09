#include "wiremessage.h"
#include "willprotocol.h"

#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <variant>
#include <vector>


namespace {


template<typename T>
void assert_roundtrip(const T& message)
{
	const will::WireMessage original{message};
	const auto encoded = will::encode(original);
	const auto decoded = will::decode(encoded);
	assert(decoded);
	const auto* roundtripped = std::get_if<T>(&*decoded);
	assert(roundtripped);
	assert(*roundtripped == message);
}


template<typename T>
bool decodes_as(const std::vector<char>& payload)
{
	const auto message = will::decode(payload);
	return message && std::holds_alternative<T>(*message);
}


} // namespace


int main()
{
	using namespace will;

	{
		const auto v = encode(UserChat{"hi"});
		assert(v.size() == 3);
		assert(static_cast<unsigned char>(v[0]) == static_cast<unsigned char>(WireMessageType::UserChat));
		assert(v[1] == 'h' && v[2] == 'i');
	}

	{
		const auto v = encode(UserChat{""});
		assert(v.size() == 1);
		assert(static_cast<unsigned char>(v[0]) == static_cast<unsigned char>(WireMessageType::UserChat));
	}

	{
		const auto a = encode(ServerReceiptAck{});
		assert(decodes_as<ServerReceiptAck>(a));
		assert(a.size() == 1);
	}

	assert(!is_valid_client_to_server_payload({}));
	assert(is_valid_client_to_server_payload(encode(UserChat{"x"})));
	assert(is_valid_client_to_server_payload(encode(UserChat{""})));
	assert(!is_valid_client_to_server_payload(encode(ServerReceiptAck{})));
	assert(!is_valid_client_to_server_payload({'\0'}));

	{
		const auto login = encode(LoginRequestPayload{"alice", "secret"});
		assert(decodes_as<LoginRequestPayload>(login));
		assert(is_valid_client_to_server_payload(login));
		const auto login_message = decode(login);
		assert(login_message);
		const auto* parsed_login = std::get_if<LoginRequestPayload>(&*login_message);
		assert(parsed_login);
		assert(parsed_login->login == "alice");
		assert(parsed_login->password == "secret");
	}

	{
		const auto ok = encode(LoginResponsePayload{true, "session-token", 0});
		assert(decodes_as<LoginResponsePayload>(ok));
		const auto ok_message = decode(ok);
		assert(ok_message);
		const auto* parsed_ok = std::get_if<LoginResponsePayload>(&*ok_message);
		assert(parsed_ok);
		assert(parsed_ok->success);
		assert(parsed_ok->token == "session-token");
	}

	{
		const auto fail = encode(LoginResponsePayload{
		    false, "", static_cast<std::uint8_t>(LoginError::InvalidCredentials)});
		const auto fail_message = decode(fail);
		assert(fail_message);
		const auto* parsed_fail = std::get_if<LoginResponsePayload>(&*fail_message);
		assert(parsed_fail);
		assert(!parsed_fail->success);
		assert(parsed_fail->error_code == static_cast<std::uint8_t>(LoginError::InvalidCredentials));
	}

	{
		const auto bind = encode(BindToken{"session-token"});
		assert(decodes_as<BindToken>(bind));
		assert(is_valid_client_to_server_payload(bind));
		const auto bind_message = decode(bind);
		assert(bind_message);
		const auto* parsed_bind = std::get_if<BindToken>(&*bind_message);
		assert(parsed_bind);
		assert(parsed_bind->token == "session-token");
	}

	{
		const auto auth_required = encode(AuthRequired{});
		assert(decodes_as<AuthRequired>(auth_required));
	}

	{
		const auto request = encode(HistoryRequest{50});
		assert(decodes_as<HistoryRequest>(request));
		assert(is_valid_client_to_server_payload(request));
		const auto request_message = decode(request);
		assert(request_message);
		const auto* parsed_request = std::get_if<HistoryRequest>(&*request_message);
		assert(parsed_request);
		assert(parsed_request->limit == 50u);
	}

	{
		const auto item = encode(HistoryItemPayload{42, true, "stored"});
		assert(decodes_as<HistoryItemPayload>(item));
		const auto item_message = decode(item);
		assert(item_message);
		const auto* parsed = std::get_if<HistoryItemPayload>(&*item_message);
		assert(parsed);
		assert(parsed->message_id == 42u);
		assert(parsed->is_mine);
		assert(parsed->body == "stored");
	}

	{
		const auto end = encode(HistoryEnd{});
		assert(decodes_as<HistoryEnd>(end));
	}

	{
		const auto v = encode(UserChat{"hi"});
		const std::string line = format_for_log(v);
		assert(line.find("UserChat") != std::string::npos);
		assert(line.find("hi") != std::string::npos);
	}

	assert(format_for_log(encode(ServerReceiptAck{})) == "ServerReceiptAck");
	assert(format_for_log(encode(HistoryEnd{})) == "HistoryEnd");

	{
		const auto payload = encode(UserChat{"ping"});
		const auto frame = TcpFrame::encode(payload);
		assert(frame.size() == 4 + payload.size());
		std::array<unsigned char, 4> header{};
		std::memcpy(header.data(), frame.data(), header.size());
		const auto len = TcpFrame::read_u32_be(header);
		assert(len == payload.size());
		assert(frame[4] == payload[0]);
	}

	assert_roundtrip(UserChat{"roundtrip"});
	assert_roundtrip(UserChat{""});
	assert_roundtrip(ServerReceiptAck{});
	assert_roundtrip(HistoryRequest{50});
	assert_roundtrip(HistoryItemPayload{42, true, "stored"});
	assert_roundtrip(HistoryItemPayload{7, false, ""});
	assert_roundtrip(HistoryEnd{});
	assert_roundtrip(LoginRequestPayload{"alice", "secret"});
	assert_roundtrip(LoginResponsePayload{true, "session-token", 0});
	assert_roundtrip(LoginResponsePayload{
	    false, "", static_cast<std::uint8_t>(LoginError::InvalidCredentials)});
	assert_roundtrip(LoginResponsePayload{
	    false, "", static_cast<std::uint8_t>(LoginError::ExpiredToken)});
	assert_roundtrip(BindToken{"session-token"});
	assert_roundtrip(AuthRequired{});

	return EXIT_SUCCESS;
}
