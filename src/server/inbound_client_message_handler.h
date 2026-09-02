#pragma once

#include "infra/transport/messenger.pb.h"

#include "session_id.h"


namespace will {


class ProtocolAdapter;


class InboundClientMessageHandler final {
public:
    InboundClientMessageHandler(ProtocolAdapter& adapter, SessionId session_id);

    void on(const v1::ClientEvent& event);

private:
    void on_bound_event(const v1::ClientEvent& event);
    void on_unbound_event(const v1::ClientEvent& event);

    ProtocolAdapter& adapter_;
    SessionId session_id_;
};


} // namespace will
