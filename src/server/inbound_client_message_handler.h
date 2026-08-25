#pragma once

#include "proto/messenger.pb.h"

#include <cstdint>


namespace will {


class ProtocolAdapter;


class InboundClientMessageHandler final {
public:
    InboundClientMessageHandler(ProtocolAdapter& adapter, std::uint64_t session_id);

    void on(const v1::ClientEvent& event);

private:
    void on_bound_event(const v1::ClientEvent& event);
    void on_unbound_event(const v1::ClientEvent& event);

    ProtocolAdapter& adapter_;
    std::uint64_t session_id_;
};


} // namespace will
