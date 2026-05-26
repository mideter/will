#include "willmessage.h"

#include <cassert>
#include <cstdlib>
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

	return EXIT_SUCCESS;
}
