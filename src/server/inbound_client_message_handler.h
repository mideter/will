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
    ProtocolAdapter& adapter_;
    std::uint64_t connection_id_;
};


} // namespace will
