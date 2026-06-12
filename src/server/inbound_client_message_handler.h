#pragma once

#include "wiremessage.h"

#include <cstdint>


namespace will {


class ProtocolAdapter;


class InboundClientMessageHandler final {
public:
    InboundClientMessageHandler(ProtocolAdapter& adapter, std::uint64_t connection_id);

    void on(const ClientMessage& message);

private:
    void on_bound_message(const ClientMessage& message);
    void on_awaiting_code_message(const ClientMessage& message);
    void on_unbound_message(const ClientMessage& message);

    ProtocolAdapter& adapter_;
    std::uint64_t connection_id_;
};


} // namespace will
