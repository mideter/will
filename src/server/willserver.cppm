export module will.server.willserver;

export import will.persistence.sqlite_persistence_bundle;
export import will.server.grpcserver;
export import will.server.serverconfig;

export namespace will {


class WillServer {
public:
    static constexpr const char* Version = "6.0.0";

    explicit WillServer(ServerConfig config = {});

    void run();

private:
    static void log_startup(const ServerConfig& config);

    ServerConfig config_;
    SqlitePersistenceBundle persistence_;
    GrpcMessengerServer server_;
};


} // namespace will
