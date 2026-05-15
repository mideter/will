#include "frameenvelope.h"

#include <cassert>
#include <cstdlib>
#include <string>
#include <vector>


int main()
{
	using namespace will;

	// encode/decode chat roundtrip
	{
		const auto frame = FrameEnvelope::encode_chat(0xdeadbeefcafeu, "hello");
		assert(frame.size() == FrameEnvelope::ChatMinPayloadBytes + 5);
		std::uint64_t id = 0;
		std::string_view body;
		assert(FrameEnvelope::try_decode_chat(frame, id, body));
		assert(id == 0xdeadbeefcafeu);
		assert(body == "hello");
	}

	// empty chat body
	{
		const auto frame = FrameEnvelope::encode_chat(1, "");
		assert(frame.size() == FrameEnvelope::ChatMinPayloadBytes);
		std::uint64_t id = 0;
		std::string_view body;
		assert(FrameEnvelope::try_decode_chat(frame, id, body));
		assert(id == 1);
		assert(body.empty());
	}

	// server ack roundtrip
	{
		const auto frame = FrameEnvelope::encode_server_ack(42, 3, 2);
		assert(frame.size() == FrameEnvelope::ServerAckPayloadBytes);
		std::uint64_t id = 0;
		std::uint32_t t = 0, s = 0;
		assert(FrameEnvelope::try_decode_server_ack(frame, id, t, s));
		assert(id == 42 && t == 3 && s == 2);
	}

	// legacy payload is not v2 chat
	{
		std::vector<char> legacy{'A', ':', ' ', 'x'};
		assert(!FrameEnvelope::looks_like_v2_chat(legacy));
		std::uint64_t id = 0;
		std::string_view body;
		assert(!FrameEnvelope::try_decode_chat(legacy, id, body));
	}

	// wrong kind is not chat
	{
		auto bad = FrameEnvelope::encode_server_ack(1, 0, 0);
		std::uint64_t id = 0;
		std::string_view body;
		assert(!FrameEnvelope::try_decode_chat(bad, id, body));
	}

	// looks_like helpers
	{
		auto chat = FrameEnvelope::encode_chat(9, "z");
		assert(FrameEnvelope::looks_like_v2_chat(chat));
		auto ack = FrameEnvelope::encode_server_ack(9, 1, 1);
		assert(FrameEnvelope::looks_like_server_ack(ack));
		assert(!FrameEnvelope::looks_like_server_ack(chat));
	}

	return EXIT_SUCCESS;
}
