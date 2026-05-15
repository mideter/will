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
		const auto v = WillMessage::encode_user_chat("hi");
		const std::string line = WillMessage::format_payload_for_log(v);
		assert(line.find("UserChat") != std::string::npos);
		assert(line.find("hi") != std::string::npos);
	}

	assert(WillMessage::format_payload_for_log(WillMessage::encode_server_receipt_ack()) == "ServerReceiptAck");

	return EXIT_SUCCESS;
}
