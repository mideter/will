#pragma once

#include <memory>
#include <string>
#include <vector>

#include "wiremessage_base.h"
#include "wiremessage_client.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"


namespace will {


std::vector<char> encode(const WireMessageBase& message);
std::unique_ptr<WireMessageBase> decode_message(const std::vector<char>& payload);
std::unique_ptr<ClientMessage> decode_client_message(const std::vector<char>& payload);
std::unique_ptr<ServerMessage> decode_server_message(const std::vector<char>& payload);

/** Structurally valid client → server types (auth gating is enforced in the adapter). */
bool is_valid_client_to_server_payload(const std::vector<char>& payload) noexcept;

std::string format_for_log(const WireMessageBase& message);
std::string format_for_log(const std::vector<char>& payload);


} // namespace will
