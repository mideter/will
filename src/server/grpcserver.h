#pragma once

#include "connectionaccountstore.h"
#include "protocoladapter.h"
#include "serverconfig.h"
#include "sessionregistry.h"

#include "ports/messenger_persistence.h"

#include "proto/messenger.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <cstddef>
#include <memory>


namespace will {


class MessengerService final : public v1::Messenger::Service {
public:
    MessengerService(ProtocolAdapter& adapter, SessionRegistry& registry, ConnectionAccountStore& account_store,
                     std::size_t max_connections);

    grpc::Status Session(grpc::ServerContext* context,
                         grpc::ServerReaderWriter<v1::ServerEvent, v1::ClientEvent>* stream) override;

private:
    ProtocolAdapter& adapter_;
    SessionRegistry& registry_;
    ConnectionAccountStore& account_store_;
    std::size_t max_connections_;
};


class GrpcMessengerServer {
public:
    GrpcMessengerServer(ServerConfig config, domain::MessengerPersistence persistence);

    void run();

    void request_stop();

private:
    ServerConfig config_;
    ConnectionAccountStore account_store_;
    SessionRegistry registry_;
    ProtocolAdapter protocol_adapter_;
    MessengerService service_;
    std::unique_ptr<grpc::Server> server_;
    std::atomic<bool> stopping_{false};
};


} // namespace will
