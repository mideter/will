#pragma once

#include <memory>
#include <string>
#include <vector>

#include "wiremessage_base.h"
#include "wiremessage_client.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"


namespace will {


std::vector<char> encode(const WireMessage& message);
std::unique_ptr<WireMessage> decode_message(const std::vector<char>& payload);
std::unique_ptr<ClientMessage> decode_client_message(const std::vector<char>& payload);
std::unique_ptr<ServerMessage> decode_server_message(const std::vector<char>& payload);

std::string format_for_log(const std::vector<char>& payload);


} // namespace will
