module;

#include <memory>

export module will.server.grpcserver;

export import will.server.serverconfig;

export namespace will {


class GrpcMessengerServer {
public:
    GrpcMessengerServer(ServerConfig config, void* messages, void* users);
    ~GrpcMessengerServer();

    GrpcMessengerServer(const GrpcMessengerServer&) = delete;
    GrpcMessengerServer& operator=(const GrpcMessengerServer&) = delete;

    void run();

    void request_stop();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};


} // namespace will
